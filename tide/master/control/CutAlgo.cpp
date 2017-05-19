#include "CutAlgo.h"
#include "limits.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"

using CutPointPtr = boost::shared_ptr<CutPoint>;

CutAlgo::CutAlgo(const DisplayGroup &group)
    : LayoutPolicy(group)
{
}

QRectF CutAlgo::getFocusedCoord(const ContentWindow &window) const
{
    updateFocusedCoord(_group.getFocusedWindows());
    return window.getCoordinates();
}

void CutAlgo::updateFocusedCoord(const ContentWindowSet &windows) const
{
    // TODO check session 4-equal with blue end with segmentation fault
    if (windows.size() == 0)
    {
        return;
    }
    else
    {
        CutGrid grid = CutGrid();
        std::vector<ContentWindowPtr> windowVec = _sortByMaxRatio(windows);
        grid.insertWindowAtPoint(grid.getPossibleInsertionPointsForWindow(
                                     windowVec[0])[0],
                                 windowVec[0]);
        QRectF enclosingRectangle =
            _defineFirstEnclosingRectangle(windowVec[0]);
        windowVec.erase(windowVec.begin());
        while (windowVec.size() > 0)
        {
            std::vector<CutPointPtr> insertionPoints =
                grid.getPossibleInsertionPointsForWindow(windowVec[0]);
            CutPointPtr bestPoint =
                _findBestInsertionPoint(insertionPoints, enclosingRectangle,
                                        windowVec[0]);
            grid.insertWindowAtPoint(bestPoint, windowVec[0]);
            enclosingRectangle =
                _computeRectIfInsertion(bestPoint, enclosingRectangle,
                                        windowVec[0]);
            windowVec.erase(windowVec.begin());
        }
        grid.balance(_getAvailableSpace());
    }
}

// Not sure how to do this, take first that does not add space
// TODO modify when use recursion in recursive way
CutPointPtr CutAlgo::_findBestInsertionPoint(std::vector<CutPointPtr> &points,
                                             QRectF enclosingRectangle,
                                             ContentWindowPtr window) const
{
    assert(points.size() > 0);
    size_t currentIndex = 0;
    size_t minIndex = 0;
    int minWidthEnclosingRectangle = INT_MAX;
    qreal minPointL1Norm = 1500000;
    while (currentIndex < points.size())
    {
        CutPointPtr currentPoint = points[currentIndex];
        if (_isInRectangle(currentPoint, enclosingRectangle, window))
        {
            return points[currentIndex];
        }
        else
        {
            QRectF wouldBeRect =
                _computeRectIfInsertion(points[currentIndex],
                                        enclosingRectangle, window);
            if (wouldBeRect.width() < minWidthEnclosingRectangle)
            {
                minWidthEnclosingRectangle = wouldBeRect.width();
                minIndex = currentIndex;
                minPointL1Norm =
                    points[currentIndex]->getX() + points[currentIndex]->getY();
            }
            else if (wouldBeRect.width() == minWidthEnclosingRectangle &&
                     points[currentIndex]->getX() +
                             points[currentIndex]->getY() <
                         minPointL1Norm)
            {
                minWidthEnclosingRectangle = wouldBeRect.width();
                minIndex = currentIndex;
                minPointL1Norm =
                    points[currentIndex]->getX() + points[currentIndex]->getY();
            }
        }
        currentIndex++;
    }
    return points[minIndex];
}

bool CutAlgo::_isInRectangle(CutPointPtr point,
                             const QRectF &enclosingRectangle,
                             ContentWindowPtr window) const
{
    return (point->getX() + LayoutPolicy::rectWithMargins(window).width() <
                enclosingRectangle.width() &&
            (point->getY() + LayoutPolicy::rectWithMargins(window).height() <
             enclosingRectangle.height()));
}

QRectF CutAlgo::_computeRectIfInsertion(CutPointPtr point,
                                        const QRectF &enclosingRectangle,
                                        ContentWindowPtr window) const
{
    QRectF rectByAddingWindow =
        QRectF(0.0, 0.0, point->getX() + rectWithMargins(window).width(),
               point->getY() + rectWithMargins(window).height());
    QRectF newEnclosingRectangle =
        _getEnclosingRectangleOfRatio(rectByAddingWindow, enclosingRectangle);
    // check if the new enclosing rectangle is bigger or smaller than the
    // previous one
    if (std::max(newEnclosingRectangle.width() / rectByAddingWindow.width(),
                 newEnclosingRectangle.height() /
                     rectByAddingWindow.height()) >= 1.0)
    {
        return newEnclosingRectangle;
    }
    else
    {
        return enclosingRectangle;
    }
}

QRectF CutAlgo::_defineFirstEnclosingRectangle(ContentWindowPtr window) const
{
    QRectF available_space = _getAvailableSpace();
    QRectF rect =
        _getEnclosingRectangleOfRatio(rectWithMargins(window), available_space);
    rect.moveLeft(0.0);
    rect.moveTop(0.0);
    return rect;
}

QRectF CutAlgo::_getEnclosingRectangleOfRatio(const QRectF &rectToEnclose,
                                              const QRectF &rectRatio) const
{
    qreal scaleFactor = std::max(rectToEnclose.width() / rectRatio.width(),
                                 rectToEnclose.height() / rectRatio.height());
    return QRectF(rectToEnclose.left(), rectToEnclose.top(),
                  rectRatio.width() * scaleFactor,
                  rectRatio.height() * scaleFactor);
}
