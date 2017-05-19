#include "CutGrid.h"
#include "Cut.h"
#include "CutRect.h"
#include "LayoutPolicy.h"
#include "scene/ContentWindow.h"
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
    QRectF rectWithMargins = LayoutPolicy::rectWithMargins(window);
    rectWithMargins.moveTopLeft(QPoint(point->getX(), point->getY()));
    return !(_intersectWithPreviousWindows(rectWithMargins));
}

bool CutGrid::insertWindowAtPoint(CutPointPtr point, ContentWindowPtr window)
{
    QRectF rectWithMargins = LayoutPolicy::rectWithMargins(window);
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
    QRectF bleuh = QRectF(availableSpace.toRect());
    std::cout << bleuh.width() << std::endl;
    //    _scaleCuts(availableSpace);
    //    _setOrderOfCuts();
    //    std::vector<std::vector<int>> matrixOfIds(
    //        widthCuts.size() - 1, std::vector<int>(heightCuts.size() - 1,
    //        -1));
    //    _fillMatrix(matrixOfIds);
    //    for (size_t windowIndex = 0; windowIndex < windowsAdded.size();
    //         windowIndex++)
    //    {
    //        std::vector<boost::shared_ptr<size_t>> indices =
    //            _getIndicesInMatrix(windowsAdded[windowIndex]);
    //        _giveNewBoundsToWindow(indices, windowIndex);
    //        //_findBiggerCoordsForId(windowIndex, matrixOfIds, indices);
    //        // TODO fix this, must find the space it occupies and add the cuts
    //        /*for (CutPtr cut : _giveNewBoundsToWindow(indices, windowIndex))
    //        {
    //            _addCut(cut);
    //        }*/
    //    }
    _updateWindows();
}

void CutGrid::_updateWindows()
{
    for (CutRectPtr rect : windowsAdded)
    {
        rect->updateWindowSize();
    }
}

std::vector<CutPtr> CutGrid::_giveNewBoundsToWindow(
    const std::vector<boost::shared_ptr<size_t>>& indices, int windowIndex)
{
    return windowsAdded[windowIndex]->giveNewBounds(widthCuts[*indices[0]],
                                                    widthCuts[*indices[1]],
                                                    heightCuts[*indices[2]],
                                                    heightCuts[*indices[3]]);
}

// format of indices is beginWidthIndex, endWidthIndex(not included),
// beginHeigthIndex, endHeightIndex (not included)
// TODO check if augment need to modify matrixOfIds, check if id is needed
void CutGrid::_findBiggerCoordsForId(
    int id, std::vector<std::vector<int>>& matrixOfIds,
    std::vector<boost::shared_ptr<size_t>>& indices) const
{
    _augmentRight(id, matrixOfIds, indices);
    _augmentLeft(id, matrixOfIds, indices);
    _augmentBottom(id, matrixOfIds, indices);
    _augmentTop(id, matrixOfIds, indices);
}

void CutGrid::_augmentRight(
    int id, std::vector<std::vector<int>>& matrixOfIds,
    std::vector<boost::shared_ptr<size_t>>& indices) const
{
    for (size_t j = *indices[2]; j < *indices[3]; j++)
    {
        if (*indices[1] >= matrixOfIds.size() ||
            !(matrixOfIds[*indices[1]][j] == -1))
        {
            return;
        }
    }
    indices[1] = boost::make_shared<size_t>(*indices[1] + 1);
    _augmentRight(id, matrixOfIds, indices);
}

void CutGrid::_augmentLeft(
    int id, std::vector<std::vector<int>>& matrixOfIds,
    std::vector<boost::shared_ptr<size_t>>& indices) const
{
    for (size_t j = *indices[2]; j < *indices[3]; j++)
    {
        if (*indices[0] == 0 || matrixOfIds[*indices[0] - 1][j] != -1)
        {
            return;
        }
    }
    indices[0] = boost::make_shared<size_t>(*indices[0] - 1);
    _augmentLeft(id, matrixOfIds, indices);
}

void CutGrid::_augmentTop(int id, std::vector<std::vector<int>>& matrixOfIds,
                          std::vector<boost::shared_ptr<size_t>>& indices) const
{
    for (size_t i = *indices[0]; i < *indices[1]; i++)
    {
        if (*indices[2] == 0 || matrixOfIds[i][*indices[2] - 1] != -1)
        {
            return;
        }
    }
    indices[2] = boost::make_shared<size_t>(*indices[2] - 1);
    _augmentTop(id, matrixOfIds, indices);
}

void CutGrid::_augmentBottom(
    int id, std::vector<std::vector<int>>& matrixOfIds,
    std::vector<boost::shared_ptr<size_t>>& indices) const
{
    for (size_t i = *indices[0]; i < *indices[1]; i++)
    {
        if (*indices[3] >= matrixOfIds[0].size() ||
            matrixOfIds[i][*indices[3]] != -1)
        {
            return;
        }
    }
    indices[3] = boost::make_shared<size_t>(*indices[3] + 1);
    _augmentBottom(id, matrixOfIds, indices);
}

void CutGrid::_scaleCuts(const QRectF& availableSpace)
{
    qreal scale_width =
        availableSpace.width() / widthCuts[widthCuts.size() - 1]->getX();
    qreal scale_height =
        availableSpace.height() / heightCuts[heightCuts.size() - 1]->getY();
    for (CutPtr cut : widthCuts)
    {
        cut->scale(scale_width, availableSpace.left());
    }
    for (CutPtr cut : heightCuts)
    {
        cut->scale(scale_height, availableSpace.top());
    }
}

void CutGrid::_fillMatrix(std::vector<std::vector<int>>& initialVector) const
{
    for (int id = 0; id < int(windowsAdded.size()); id++)
    {
        size_t widthBegin = windowsAdded[id]->beginOrderWidth();
        size_t widthEnd = windowsAdded[id]->endOrderWidth();
        size_t heightBegin = windowsAdded[id]->beginOrderHeight();
        size_t heightEnd = windowsAdded[id]->endOrderHeight();
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
}

void CutGrid::_setOrderOfCuts() const
{
    for (size_t order = 0; order < widthCuts.size(); order++)
    {
        widthCuts[order]->setOrder(order);
    }
    for (size_t order = 0; order < heightCuts.size(); order++)
    {
        heightCuts[order]->setOrder(order);
    }
}

std::vector<boost::shared_ptr<size_t>> CutGrid::_getIndicesInMatrix(
    const CutRectPtr cutRect) const
{
    return std::vector<boost::shared_ptr<size_t>>{
        boost::make_shared<size_t>(cutRect->beginOrderWidth()),
        boost::make_shared<size_t>(cutRect->endOrderWidth()),
        boost::make_shared<size_t>(cutRect->beginOrderHeight()),
        boost::make_shared<size_t>(cutRect->endOrderHeight())};
}
