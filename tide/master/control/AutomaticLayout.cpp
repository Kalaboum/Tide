#include "AutomaticLayout.h"
#include "control/ContentWindowController.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "types.h"
#include "CanvasTree.h"


qreal AutomaticLayout::_computeMaxRatio(ContentWindowPtr window) const {
    return std::max(window->width()/OPTIMAL_WIDTH, window->height()/OPTIMAL_HEIGHT);
}

void AutomaticLayout::_dichotomicInsert(ContentWindowPtr window,ContentWindowPtrs& windowVec) const{
    int a = 0;
    int b = windowVec.size() -1;
    while(b > a){
        if(_computeMaxRatio(windowVec[(a + b)/2]) > _computeMaxRatio(window)){
            b = (a + b) / 2;
        }
        else {
            a = (a + b) /2;
        }
    }
    windowVec.insert(windowVec.begin()+a, window);
}

//Optimal height and width is initialized to be display group width and height, that can be changed if needed
AutomaticLayout::AutomaticLayout(const DisplayGroup& group) : LayoutPolicy(group), OPTIMAL_WIDTH(group.width()), OPTIMAL_HEIGHT(group.height())
{
}


QRectF AutomaticLayout::getFocusedCoord( const ContentWindow& window ) const
{
    return _getFocusedCoord( window, _group.getFocusedWindows( ));
}

//TODO ask if const necessary, would like to add fields that change
void AutomaticLayout::updateFocusedCoord( const ContentWindowSet& windows ) const
{
    std::cerr << "optimal width in automaticLayout is : " << OPTIMAL_WIDTH << std::endl;
    CanvasTree layoutTree = CanvasTree(_sortByMaxRatio(windows), OPTIMAL_WIDTH, OPTIMAL_HEIGHT);
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
