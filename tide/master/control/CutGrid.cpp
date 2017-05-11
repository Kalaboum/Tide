#include "CutGrid.h"
#include "Cut.h"
#include "LayoutPolicy.h"

using CutPointPtr = boost::shared_ptr<CutPoint>;
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
    rectWithMargins.setTopRight(QPoint(point->getX(), point->getY()));
    return !(_intersectWithPreviousWindows(rectWithMargins));
}

bool CutGrid::insertWindowAtPoint(CutPointPtr point, ContentWindowPtr window)
{
    QRectF rectWithMargins = LayoutPolicy::_addMargins(window);
    rectWithMargins.setTopRight(QPoint(point->getX(), point->getY()));
    if (_intersectWithPreviousWindows(rectWithMargins))
    {
        return false;
    }
    else
    {
        // TODO deal with insertion Points madness
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
    // TODO check if we can assert pointer equal (check also the libs for
    // vector)
    for (size_t i = 0; i < insertionPoints.size(); i++)
    {
        if (insertionPoints[i] == point)
        {
            // TODO pop list insertionPoints
            return true;
        }
    }
    return false;
}

QRectF CutGrid::currentSize() const
{
    return QRectF(0.0, 0.0, widthCuts[widthCuts.size() - 1],
                  heightCuts[heightCuts.size() - 1]);
}

std::vector<CutPointPtr> CutGrid::getPossibleInsertionPointsForWindow(
    ContentWindowPtr window)
{
    std::vector<CutPointPtr> possiblePoints;
    for (CutPointPtr& point : insertionPoints)
    {
        if (isPossibleInsertWindowAtPoint(point, window))
        {
            possiblePoints.push_back(point);
        }
    }
    return possiblePoints;
}
