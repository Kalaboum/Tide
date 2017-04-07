#include "AutomaticLayout.h"
#include "control/ContentWindowController.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "types.h"
#include "CanvasTree.h"
#include "scene/ContentType.h"


AutomaticLayout::AutomaticLayout(const DisplayGroup& group) : LayoutPolicy(group)
{

}
AutomaticLayout::AutomaticLayout(const DisplayGroup& group, bool separateMovies) : LayoutPolicy(group), _separateMovies(separateMovies)
{
}

qreal AutomaticLayout::_computeMaxRatio(ContentWindowPtr window) const {
    return std::max(window->width()/_getAvailableSpace().width(), window->height()/_getAvailableSpace().height());
}

void AutomaticLayout::_dichotomicInsert(ContentWindowPtr window,ContentWindowPtrs& windowVec) const{
    if(windowVec.size() == 0){
        windowVec.push_back(window);
    }
    else {
        size_t a = 0;
        size_t b = windowVec.size();
        while(b - a > 1){
            if(_computeMaxRatio(windowVec[(a + b)/2]) > _computeMaxRatio(window)){
                b = (a + b) / 2;
            }
            else {
                a = (a + b) /2;
            }
        }
        if(_computeMaxRatio(windowVec[a]) < _computeMaxRatio(window)){
            windowVec.insert(windowVec.begin()+a, window);
        }else if(b < windowVec.size() && _computeMaxRatio(windowVec[b]) > _computeMaxRatio(window)){
            windowVec.insert(windowVec.begin() + b + 1, window);
        }else {
            windowVec.insert(windowVec.begin() + b, window);
        }
    }
}



QRectF AutomaticLayout::getFocusedCoord( const ContentWindow& window ) const
{
    return _getFocusedCoord( window, _group.getFocusedWindows( ));
}

void AutomaticLayout::updateFocusedCoord( const ContentWindowSet& windows ) const
{
    if(_separateMovies){
        std::vector<ContentWindowSet> separatedContent = _separateContent(windows);
        if(separatedContent[1].size() == 0){
            CanvasTree layoutTree = CanvasTree(_sortByMaxRatio(windows), _getAvailableSpace());
            layoutTree.updateFocusCoordinates();
        }
        else {
        QRectF availableSpace = _getAvailableSpace();
        qreal areaOfMovies = _getTotalArea(separatedContent[1]);
        qreal areaOfOther = _getTotalArea(separatedContent[0]);
        qreal widthForOtherContent = availableSpace.width() * (areaOfOther / (areaOfMovies + areaOfOther));
        QRectF availableSpaceForOther = QRectF(availableSpace.left(), availableSpace.top(), widthForOtherContent
                                               , availableSpace.height());
        QRectF availableSpaceForMovies = QRectF(availableSpace.left() + availableSpaceForOther.width(), availableSpace.top(),
                                                availableSpace.width() - availableSpaceForOther.width(), availableSpace.height());
        CanvasTree layoutTreeMovies = CanvasTree(_sortByMaxRatio(separatedContent[1]), availableSpaceForMovies);
        CanvasTree layoutTreeOther = CanvasTree(_sortByMaxRatio(separatedContent[0]), availableSpaceForOther);
        layoutTreeMovies.updateFocusCoordinates();
        layoutTreeOther.updateFocusCoordinates();
        }
    }
    else {
        CanvasTree layoutTree = CanvasTree(_sortByMaxRatio(windows), _getAvailableSpace());
        layoutTree.updateFocusCoordinates();
    }
}

std::vector<ContentWindowSet> AutomaticLayout::_separateContent(const ContentWindowSet& windows) const{
    ContentWindowSet movies;
    ContentWindowSet other;
    for (const ContentWindowPtr& window : windows){
        if(window->getContentPtr()->getType() == CONTENT_TYPE_MOVIE){
            movies.insert(window);
        }else {
            other.insert(window);
        }
    }
    std::vector<ContentWindowSet> myVect;
    myVect.push_back(other);
    myVect.push_back(movies);
    return myVect;
}

qreal AutomaticLayout::_getTotalArea(const ContentWindowSet& windows) const{
    qreal areaCount = 0.0;
    for (const ContentWindowPtr& window : windows){
        QSize preferredDimensions = window->getContentPtr()->getPreferredDimensions();
        areaCount +=  preferredDimensions.width() * preferredDimensions.height() ;
    }
    return areaCount;
}

QRectF AutomaticLayout::_getFocusedCoord(const ContentWindow& window, const ContentWindowSet& windows) const
{
    updateFocusedCoord(windows);
    return window.getCoordinates();

}

ContentWindowPtrs AutomaticLayout::_sortByMaxRatio(const ContentWindowSet& windows) const
{
    std::vector< ContentWindowPtr > windowVec;
    for(auto& window : windows)
    {
        _dichotomicInsert(window, windowVec);
    }
    return windowVec;
}
