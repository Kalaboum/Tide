/*********************************************************************/
/* Copyright (c) 2017, EPFL/Blue Brain Project                       */
/*                     Pawel Podhajski <pawel.podhajski@epfl.ch>     */
/*                     Raphael Dumusc <raphael.dumusc@epfl.ch>       */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of Ecole polytechnique federale de Lausanne.          */
/*********************************************************************/

#include "FileReceiver.h"

#include "json.h"
#include "log.h"
#include "scene/ContentFactory.h"

#include <zeroeq/http/helpers.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

using namespace zeroeq;

namespace
{
inline QString _urlEncode(const QString& filename)
{
    return QUrl(filename).fileName(QUrl::FullyEncoded);
}

inline QString _urlDecode(const QString& filename)
{
    return QUrl(filename).fileName(QUrl::FullyDecoded);
}

QString _getAvailableFileName(const QFileInfo& fileInfo)
{
    QString filename = fileInfo.fileName();

    int nSuffix = 0;
    while (QFile(QDir::tempPath() + "/" + filename).exists())
    {
        filename = QString("%1_%2.%3")
                       .arg(fileInfo.baseName(), QString::number(++nSuffix),
                            fileInfo.suffix());
    }
    return filename;
}

std::future<http::Response> _makeResponse(const http::Code code,
                                          const QString& key,
                                          const QString& info)
{
    const auto body = json::toString(QJsonObject{{key, info}});
    return make_ready_response(code, body, "application/json");
}
}

std::future<http::Response> FileReceiver::prepareUpload(
    const http::Request& request)
{
    const auto obj = json::toObject(request.body);
    if (obj.empty())
        return make_ready_response(http::Code::BAD_REQUEST);

    const auto filename = obj["filename"].toString();
    if (filename.contains('/'))
        return make_ready_response(http::Code::NOT_SUPPORTED);

    const auto position = QPointF{obj["x"].toDouble(), obj["y"].toDouble()};

    const QFileInfo fileInfo(filename);
    const QString fileSuffix = fileInfo.suffix();
    if (fileSuffix.isEmpty() || fileInfo.baseName().isEmpty())
        return make_ready_response(http::Code::NOT_SUPPORTED);

    const QStringList& filters = ContentFactory::getSupportedExtensions();
    if (!filters.contains(fileSuffix.toLower()))
        return make_ready_response(http::Code::NOT_SUPPORTED);

    const auto name = _getAvailableFileName(fileInfo);
    _preparedPaths[name] = position;
    return _makeResponse(http::Code::OK, "url", _urlEncode(name));
}

std::future<http::Response> FileReceiver::handleUpload(
    const zeroeq::http::Request& request)
{
    const auto name = _urlDecode(QString::fromStdString(request.path));
    if (!_preparedPaths.contains(name))
        return _makeResponse(http::Code::FORBIDDEN, "info",
                             "upload not prepared");

    const auto filePath = QDir::tempPath() + "/" + name;
    const auto position = _preparedPaths[name];
    _preparedPaths.remove(name);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly) ||
        !file.write(request.body.c_str(), request.body.size()))
    {
        put_flog(LOG_INFO, "file not created as %s",
                 filePath.toLocal8Bit().constData());
        return _makeResponse(http::Code::INTERNAL_SERVER_ERROR, "info",
                             "could not upload");
    }
    file.close();

    put_flog(LOG_INFO, "file created as %s",
             filePath.toLocal8Bit().constData());

    auto openPromise = std::make_shared<std::promise<bool>>();
    auto openSuccess = openPromise->get_future();
    emit open(filePath, position, std::move(openPromise));
    try
    {
        if (openSuccess.get())
        {
            put_flog(LOG_INFO, "file uploaded and saved as: %s",
                     filePath.toLocal8Bit().constData());
            return _makeResponse(http::Code::CREATED, "info", "OK");
        }
    }
    catch (...)
    {
    }

    file.remove();
    put_flog(LOG_INFO, "file uploaded but could not be opened: %s",
             filePath.toLocal8Bit().constData());
    return _makeResponse(http::Code::NOT_SUPPORTED, "info",
                         "file could not be opened");
}
