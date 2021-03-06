/*********************************************************************/
/* Copyright (c) 2014, EPFL/Blue Brain Project                       */
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

#include "RenderController.h"

#include "DisplayGroupRenderer.h"
#include "TextureUploader.h"
#include "WallWindow.h"
#include "network/WallToWallChannel.h"
#include "scene/DisplayGroup.h"
#include "scene/Options.h"

RenderController::RenderController(WallWindow& window)
    : _window(window)
    , _syncDisplayGroup(boost::make_shared<DisplayGroup>(QSize()))
    , _syncOptions(boost::make_shared<Options>())
{
    _syncDisplayGroup.setCallback(std::bind(&WallWindow::setDisplayGroup,
                                            &_window, std::placeholders::_1));
    _syncMarkers.setCallback(
        std::bind(&WallWindow::setMarkers, &_window, std::placeholders::_1));
    _syncOptions.setCallback(std::bind(&WallWindow::setRenderOptions, &_window,
                                       std::placeholders::_1));

    connect(&window.getUploader(), &TextureUploader::uploaded, this,
            [this] { _needRedraw = true; }, Qt::QueuedConnection);

    connect(&window, &WallWindow::imageGrabbed, this,
            &RenderController::screenshotRendered);
}

DisplayGroupPtr RenderController::getDisplayGroup() const
{
    return _syncDisplayGroup.get();
}

void RenderController::timerEvent(QTimerEvent* qtEvent)
{
    if (qtEvent->timerId() == _renderTimer)
        _syncAndRender();
    else if (qtEvent->timerId() == _idleRedrawTimer)
        requestRender();
    else if (qtEvent->timerId() == _stopRenderingDelayTimer)
    {
        killTimer(_renderTimer);
        killTimer(_stopRenderingDelayTimer);
        _renderTimer = 0;
        _stopRenderingDelayTimer = 0;

        // Redraw screen every minute so that the on-screen clock is up to date
        if (_idleRedrawTimer == 0)
            _idleRedrawTimer = startTimer(60000 /*ms*/);
    }
}

void RenderController::requestRender()
{
    killTimer(_stopRenderingDelayTimer);
    _stopRenderingDelayTimer = 0;
    killTimer(_idleRedrawTimer);
    _idleRedrawTimer = 0;

    if (_renderTimer == 0)
        _renderTimer = startTimer(5, Qt::PreciseTimer);
}

void RenderController::_syncAndRender()
{
    WallToWallChannel& wallChannel = _window.getWallChannel();
    auto versionCheckFunc = std::bind(&WallToWallChannel::checkVersion,
                                      &wallChannel, std::placeholders::_1);
    _syncQuit.sync(versionCheckFunc);
    if (_syncQuit.get())
    {
        killTimer(_renderTimer);
        killTimer(_stopRenderingDelayTimer);
        _window.deleteLater();
        return;
    }

    _synchronizeObjects(versionCheckFunc);

    const bool grab = _syncScreenshot.get();
    if (grab)
        _syncScreenshot = SwapSyncObject<bool>{false};

    if (!_window.syncAndRender(grab) && wallChannel.allReady(!_needRedraw))
    {
        if (_stopRenderingDelayTimer == 0)
            _stopRenderingDelayTimer = startTimer(5000 /*ms*/);
    }
    else
        requestRender();

    _needRedraw = false;
}

void RenderController::updateQuit()
{
    _syncQuit.update(true);
    requestRender();
}

void RenderController::updateRequestScreenshot()
{
    _syncScreenshot.update(true);
    requestRender();
}

void RenderController::updateDisplayGroup(DisplayGroupPtr displayGroup)
{
    _syncDisplayGroup.update(displayGroup);
    requestRender();
}

void RenderController::updateOptions(OptionsPtr options)
{
    _syncOptions.update(options);
    requestRender();
}

void RenderController::updateMarkers(MarkersPtr markers)
{
    _syncMarkers.update(markers);
    requestRender();
}

void RenderController::_synchronizeObjects(const SyncFunction& versionCheckFunc)
{
    _syncScreenshot.sync(versionCheckFunc);
    _syncDisplayGroup.sync(versionCheckFunc);
    _syncMarkers.sync(versionCheckFunc);
    _syncOptions.sync(versionCheckFunc);
}
