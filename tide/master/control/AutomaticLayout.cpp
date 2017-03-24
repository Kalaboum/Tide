#include "AutomaticLayout.h"
#include "control/ContentWindowController.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "types.h"
#include "CanvasTree.h"


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

//Optimal height and width is initialized to be display group width and height, that can be changed if needed
AutomaticLayout::AutomaticLayout(const DisplayGroup& group) : LayoutPolicy(group)
{
}


QRectF AutomaticLayout::getFocusedCoord( const ContentWindow& window ) const
{
    return _getFocusedCoord( window, _group.getFocusedWindows( ));
}

//TODO ask if const necessary, would like to add fields that change
void AutomaticLayout::updateFocusedCoord( const ContentWindowSet& windows ) const
{;
    CanvasTree layoutTree = CanvasTree(_sortByMaxRatio(windows), _getAvailableSpace());
    layoutTree.updateFocusCoordinates();
}

//TODO is it necessary to redo algorithm everytime this is called ?
QRectF AutomaticLayout::_getFocusedCoord(const ContentWindow& window, const ContentWindowSet& windows) const
{
    ContentWindowPtrs windowVec = _sortByMaxRatio(windows);

    //TODO remove dummy
    return QRectF(window.width(), 0.0, 0.0, 0.0);

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
