#include "CutGrid.h"
#include "Cut.h"
#include "LayoutPolicy.h"
#include "scene/ContentWindow.h"
#include <iostream.h>
CutGrid::CutGrid()
{
    widthCuts.push_back(boost::make_shared<Cut>(Cut::widthCut(0.0)));
    heightCuts.push_back(boost::make_shared<Cut>(Cut::heightCut(0.0)));
    insertionPoints.push_back(
        boost::make_shared<CutPoint>(CutPoint(widthCuts[0], heightCuts[0])));
}
bool CutGrid::isPossibleInsertWindowAtPoint(CutPointPtr point,
                                            ContentWindowPtr window) const
{
    QRectF rectWithMargins = LayoutPolicy::_addMargins(window);
    rectWithMargins.moveTopLeft(QPoint(point->getX(), point->getY()));
    return !(_intersectWithPreviousWindows(rectWithMargins));
}

bool CutGrid::insertWindowAtPoint(CutPointPtr point, ContentWindowPtr window)
{
    QRectF rectWithMargins = LayoutPolicy::_addMargins(window);
    rectWithMargins.moveTopLeft(QPoint(point->getX(), point->getY()));
    if (_intersectWithPreviousWindows(rectWithMargins))
    {
        return false;
    }
    else
    {
        assert(_removeInsertionPoint(point));
        CutPtr newWidthCut = boost::make_shared<Cut>(
            Cut::widthCut(rectWithMargins.left() + rectWithMargins.width()));
        CutPtr newHeightCut = boost::make_shared<Cut>(
            Cut::heightCut(rectWithMargins.top() + rectWithMargins.height()));
        _addCut(newWidthCut);
        _addCut(newHeightCut);
        CutRectPtr insertedRect = boost::make_shared<CutRect>(
            CutRect::createCutRect(point->getWidthCut(), newWidthCut,
                                   point->getHeightCut(), newHeightCut,
                                   window));
        windowsAdded.push_back(insertedRect);
        CutPointPtr firstInsertionPoint = boost::make_shared<CutPoint>(
            CutPoint(point->getWidthCut(), newHeightCut));
        CutPointPtr secondInsertionPoint = boost::make_shared<CutPoint>(
            CutPoint(newWidthCut, point->getHeightCut()));
        insertionPoints.push_back(firstInsertionPoint);
        insertionPoints.push_back(secondInsertionPoint);
        return true;
    }
}

void CutGrid::_addCut(CutPtr cut)
{
    if (cut->getX() != -1.0)
    {
        widthCuts.push_back(cut);
        std::sort(widthCuts.begin(), widthCuts.end(),
                  [](CutPtr cut1, CutPtr cut2) {
                      return cut1->getX() < cut2->getX();
                  });
    }
    else
    {
        heightCuts.push_back(cut);
        std::sort(heightCuts.begin(), heightCuts.end(),
                  [](CutPtr cut1, CutPtr cut2) {
                      return cut1->getY() < cut2->getY();
                  });
    }
}

bool CutGrid::_intersectWithPreviousWindows(const QRectF& rect) const
{
    for (const CutRectPtr& cutRect : windowsAdded)
    {
        if (cutRect->intersectWith(rect))
        {
            return true;
        }
    }
    return false;
}

bool CutGrid::_removeInsertionPoint(CutPointPtr point)
{
    for (size_t i = 0; i < insertionPoints.size(); i++)
    {
        if (insertionPoints[i] == point)
        {
            insertionPoints.erase(insertionPoints.begin() + i);
            return true;
        }
    }
    return false;
}

QRectF CutGrid::currentSize() const
{
    return QRectF(0.0, 0.0, widthCuts[widthCuts.size() - 1]->getX(),
                  heightCuts[heightCuts.size() - 1]->getY());
}

std::vector<CutPointPtr> CutGrid::getPossibleInsertionPointsForWindow(
    ContentWindowPtr window) const
{
    std::vector<CutPointPtr> possiblePoints;
    for (CutPointPtr point : insertionPoints)
    {
        if (isPossibleInsertWindowAtPoint(point, window))
        {
            possiblePoints.push_back(point);
        }
    }
    return possiblePoints;
}

// TODO change this to balance it effectively
void CutGrid::balance(const QRectF& availableSpace)
{
    _scaleCuts(availableSpace);
    for (int windowIndex = 0; windowIndex < windowsAdded.size(); windowIndex++)
    {
        std::vector<std::vector<int>> matrixOfIds = _toMatrix();
        std::vector<boost::make_shared<int>> indices =
            _getIndicesInMatrix(windowsAdded[windowIndex]);
        _findBiggerCoordsForId(windowIndex, matrixOfIds, indices);
        _changeCutsWithNewIndicesOfMatrix(indices, windowIndex);
    }
    _updateWindows();
}

void CutGrid::_updateWindows()
{
    for (CutRectPtr rect : windowsAdded)
    {
        rect->updateWindowSize();
    }
}

void CutGrid::_changeCutsWithNewIndicesOfMatrix(
    const std::vector<boost::shared_ptr<int>>& indices, int windowIndex)
{
    windowsAdded[windowIndex]->changeCuts(widthCuts[indices[0]].get(),
                                          widthCuts[indices[1].get()],
                                          heightCuts[indices[2].get()],
                                          heightCuts[indices[3].get()]);
}

// format of indices is beginWidthIndex, endWidthIndex(included),
// beginHeigthIndex, endHeightIndex (included)
// TODO check if augment need to modify matrixOfIds, check if id is needed
void CutGrid::_findBiggerCoordsForId(
    int id, std::vector<std::vector<int>>& matrixOfIds,
    std::vector<boost::shared_ptr<int>> indices) const
{
    _augmentRight(id, matrixOfIds, indices);
    _augmentLeft(id, matrixOfIds, indices);
    _augmentBottom(id, matrixOfIds, indices);
    _augmentTop(id, matrixOfIds, indices);
}

void CutGrid::_augmentRight(int id, std::vector<std::vector<int>>& matrixOfIds,
                            std::vector<boost::shared_ptr<int>> indices) const
{
    for (int j = indices[2].get; j <= indices[3].get; j++)
    {
        if (indices[1].get() + 1 >= matrixOfIds.size() ||
            matrixOfIds[indices[1].get() + 1] != -1)
        {
            return;
        }
    }
    indices[1] = boost::make_shared<int>(indices[1].get() + 1);
    _augmentRight(id, matrixOfIds, indices);
}

void CutGrid::_augmentLeft(int id, std::vector<std::vector<int>>& matrixOfIds,
                           std::vector<boost::shared_ptr<int>> indices) const
{
    for (int j = indices[2].get; j <= indices[3].get; j++)
    {
        if (indices[0].get() - 1 < 0 || matrixOfIds[indices[0].get() - 1] != -1)
        {
            return;
        }
    }
    indices[0] = boost::make_shared<int>(indices[0].get() - 1);
    _augmentLeft(id, matrixOfIds, indices);
}

void CutGrid::_augmentTop(int id, std::vector<std::vector<int>>& matrixOfIds,
                          std::vector<boost::shared_ptr<int>> indices) const
{
    for (int i = indices[0].get; i <= indices[1].get; j++)
    {
        if (indices[2].get() - 1 < 0 || matrixOfIds[indices[2].get() - 1] != -1)
        {
            return;
        }
    }
    indices[2] = boost::make_shared<int>(indices[2].get() - 1);
    _augmentTop(id, matrixOfIds, indices);
}

void CutGrid::_augmentBottom(int id, std::vector<std::vector<int>>& matrixOfIds,
                             std::vector<boost::shared_ptr<int>> indices) const
{
    for (int i = indices[0].get; i <= indices[1].get; j++)
    {
        if (indices[3].get() + 1 > matrixOfIds[0].size() ||
            matrixOfIds[indices[3].get() + 1] != -1)
        {
            return;
        }
    }
    indices[3] = boost::make_shared<int>(indices[3].get() + 1);
    _augmentBottom(id, matrixOfIds, indices);
}

void CutGrid::_augmentRight(int id, std::vector<std::vector<int>>& matrixOfIds,
                            std::vector<boost::shared_ptr<int>> indices) const
{
    for (int j = indices[2].get; j <= indices[3].get; j++)
    {
        if (matrixOfIds[indices[1].get() + 1] != -1)
        {
            return;
        }
    }
    indices[1] = boost::make_shared<int>(indices[1].get() + 1);
}

void CutGrid::_scaleCuts(const QRectF& availableSpace)
{
    qreal scale_width =
        availableSpace.width() / widthCuts[widthCuts.size() - 1];
    qreal scale_height =
        availableSpace.height() / heightCuts[heightCuts.size() - 1];
    for (CutPtr cut : widthCuts)
    {
        cut->scale(scale_width);
    }
    for (CutPtr cut : heightCuts)
    {
        cut->scale(scale_height);
    }
}

std::vector<std::vector<int>> CutGrid::_toMatrix() const
{
    std::vector<std::vector<int>> initialVector(
        widthCuts.size() - 1, std::vector<int>(heightCuts.size() - 1), -1);
    _setOrderOfCuts();
    for (int id = 0; id < windowsAdded.size(); id++)
    {
        int widthBegin = windowsAdded[id]->beginOrderWidth();
        int widthEnd = windowsAdded[id]->endOrderWidth();
        int heightBegin = windowsAdded[id]->beginOrderWidth();
        int heightEnd = windowsAdded[id]->endOrderWidth();
        for (size_t width_index = widthBegin; width_index < widthEnd;
             width_index++)
        {
            for (size_t height_index = heightBegin; height_index < heightEnd;
                 height_index++)
            {
                initialVector[width_index][height_index] = id;
            }
        }
    }
    return initialVector;
}

void CutGrid::_setOrderOfCuts()
{
    for (int order = 0; order < widthCuts.size(); order++)
    {
        widthCuts[order]->setOrder(order);
    }
    for (int order = 0; order < heightCuts.size(); order++)
    {
        heightCuts[order]->setOrder(order);
    }
}

std::vector<boost::shared_ptr<int>> CutGrid::_getIndicesInMatrix(
    const CutRectPtr cutRect) const
{
    return std::vector<boost::shared_ptr<int>>{
        boost::make_shared<int>(cutRect->beginOrderWidth()),
        boost::make_shared(cutRect->endOrderWidth() - 1),
        boost::make_shared(cutRect->beginOrderHeight()),
        boost::make_shared(cutRect->endOrderWidth() - 1)};
