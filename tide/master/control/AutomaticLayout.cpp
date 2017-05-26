#include "AutomaticLayout.h"
#include "CanvasTree.h"
#include "control/ContentWindowController.h"
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "types.h"

// TODO think about window size (preferred dimensions and all)
AutomaticLayout::AutomaticLayout(const DisplayGroup& group)
    : LayoutPolicy(group)
{
}
AutomaticLayout::AutomaticLayout(const DisplayGroup& group, bool separateMovies)
    : LayoutPolicy(group)
    , _separateMovies(separateMovies)
{
}

qreal AutomaticLayout::_computeMaxRatio(ContentWindowPtr window) const
{
    return std::max(window->width() / _getAvailableSpace().width(),
                    window->height() / _getAvailableSpace().height());
}

QRectF AutomaticLayout::getFocusedCoord(const ContentWindow& window) const
{
    return _getFocusedCoord(window, _group.getFocusedWindows());
}

void AutomaticLayout::updateFocusedCoord(const ContentWindowSet& windows) const
{
    if (_separateMovies)
    {
        std::vector<ContentWindowSet> separatedContent =
            _separateContent(windows);
        if (separatedContent[1].size() == 0)
        {
            CanvasTree layoutTree =
                CanvasTree(_sortByMaxRatio(windows), _getAvailableSpace());
            layoutTree.updateFocusCoordinates();
        }
        else
        {
            QRectF availableSpace = _getAvailableSpace();
            qreal areaOfMovies = _getTotalArea(separatedContent[1]);
            qreal areaOfOther = _getTotalArea(separatedContent[0]);
            qreal widthForOtherContent =
                availableSpace.width() *
                (areaOfOther / (areaOfMovies + areaOfOther));
            QRectF availableSpaceForOther =
                QRectF(availableSpace.left(), availableSpace.top(),
                       widthForOtherContent, availableSpace.height());
            QRectF availableSpaceForMovies =
                QRectF(availableSpace.left() + availableSpaceForOther.width(),
                       availableSpace.top(),
                       availableSpace.width() - availableSpaceForOther.width(),
                       availableSpace.height());
            CanvasTree layoutTreeMovies =
                CanvasTree(_sortByMaxRatio(separatedContent[1]),
                           availableSpaceForMovies);
            CanvasTree layoutTreeOther =
                CanvasTree(_sortByMaxRatio(separatedContent[0]),
                           availableSpaceForOther);
            layoutTreeMovies.updateFocusCoordinates();
            layoutTreeOther.updateFocusCoordinates();
        }
    }
    else
    {
        ContentWindowPtrs sortedWindows = _sortByMaxRatio(windows);
        CanvasTree layoutTree = CanvasTree(sortedWindows, _getAvailableSpace());
        qreal maxOccupiedSpace = layoutTree.getOccupiedSpace();
        for (int i = 0; i < MAX_RANDOM_PERMUTATIONS; i++)
        {
            std::random_shuffle(sortedWindows.begin(), sortedWindows.end());
            CanvasTree currentTree =
                CanvasTree(sortedWindows, _getAvailableSpace());
            if (currentTree.getOccupiedSpace() > maxOccupiedSpace)
            {
                std::cout << "permutations did well" << std::endl;
                layoutTree = currentTree;
                maxOccupiedSpace = currentTree.getOccupiedSpace();
            }
        }
        layoutTree.updateFocusCoordinates();
    }
}

std::vector<ContentWindowSet> AutomaticLayout::_separateContent(
    const ContentWindowSet& windows) const
{
    ContentWindowSet movies;
    ContentWindowSet other;
    for (const ContentWindowPtr& window : windows)
    {
        if (window->getContentPtr()->getType() == CONTENT_TYPE_MOVIE)
        {
            movies.insert(window);
        }
        else
        {
            other.insert(window);
        }
    }
    std::vector<ContentWindowSet> myVect;
    myVect.push_back(other);
    myVect.push_back(movies);
    return myVect;
}

qreal AutomaticLayout::_getTotalArea(const ContentWindowSet& windows) const
{
    qreal areaCount = 0.0;
    for (const ContentWindowPtr& window : windows)
    {
        QSize preferredDimensions =
            window->getContentPtr()->getPreferredDimensions();
        areaCount += preferredDimensions.width() * preferredDimensions.height();
    }
    return areaCount;
}

QRectF AutomaticLayout::_getFocusedCoord(const ContentWindow& window,
                                         const ContentWindowSet& windows) const
{
    updateFocusedCoord(windows);
    return window.getCoordinates();
}

ContentWindowPtrs AutomaticLayout::_sortByMaxRatio(
    const ContentWindowSet& windows) const
{
    std::vector<ContentWindowPtr> windowVec;
    for (auto window : windows)
    {
        windowVec.push_back(window);
    }
    std::sort(windowVec.begin(), windowVec.end(),
              [this](ContentWindowPtr a, ContentWindowPtr b) {
                  return _computeMaxRatio(a) > _computeMaxRatio(b);
              });
    return windowVec;
}
