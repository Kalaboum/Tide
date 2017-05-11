#include "CutAlgo.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

CutAlgo::CutAlgo(const DisplayGroup &group)
    : LayoutPolicy(group)
{
    grid = CutGrid();
}

QRectF CutAlgo::getFocusedCoord(const ContentWindow &window) const
{
    updateFocusedCoord(_group.getFocusedWindows());
    window.getCoordinates();
}

void CutAlgo::updateFocusedCoord(const ContentWindowSet &windows) const
{
    if (windows.size() == 0)
    {
        return;
    }
    else
    {
        std::vector<ContentWindowPtr> windowVec = _sortByMaxRatio(windows);
        grid.insertWindowAtPoint(grid.getPossibleInsertionPointsForWindow(
                                     windowVec[0]),
                                 windowVec[0]);
        windowVec.erase(windowVec.begin());
        QRectF enclosingRectangle = ; // TODO define enclosing rectangle
        while (windowVec.size() > 0)
        {
            std::vector<CutPointPtr> insertionPoints =
                grid.getPossibleInsertionPointsForWindow(windows[0],
                                                         enclosingRectangle);
        }
    }
}

// Not sure how to do this, take first that does not add space
CutPointPtr CutAlgo::_findBestInsertionPoint(std::vector<CutPointPtr> &points,
                                             QRectF enclosingRectangle,
                                             ContentWindowPtr window) const
{
    size_t currentIndex = 0;
    // TODO define max int,int min = std::in
    int minWidthEnclosingRectangle;
    while (currentIndex < points.size())
    {
        CutPointPtr currentPoint = point[currentIndex];
        if (_isInRectangle(currentPoint, enclosingRectangle, window))
        {
            return windowVec[curentIndex];
        }
        else
        {
            QRectF wouldBeRect = _computeRectIfInsertion()
        }
    }
}

bool CutAlgo::_isInRectangle(CutPointPtr point, QRectF enclosingRectangle,
                             ContentWindowPtr window) const
{
    return (point->getX() + LayoutPolicy::_addMargins(window).width() <
                enclosingRectangle.width() &&
            (point->getY() + LayoutPolicy::_addMargins(window).height() <
             enclosingRectangle.height()));
}

QRectF CutAlgo::_computeRectIfInsertion() const
{
}
