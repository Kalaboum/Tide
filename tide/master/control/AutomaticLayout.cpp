#include "AutomaticLayout.h"
#include "control/ContentWindowController.h"
#include "scene/ContentWindow.h"
#include "scene/DisplayGroup.h"
#include "types.h"

namespace
{
const qreal INSIDE_MARGIN_RELATIVE = 0.02;
const qreal SIDEBAR_WITH_REL_TO_DISPLAYGROUP_HEIGHT = 0.3 * 0.3;
const qreal WINDOW_CONTROLS_MARGIN_PX = 200.0;
const qreal WINDOW_SPACING_PX = 80.0;
}

qreal AutomaticLayout::_computeMaxRatio(ContentWindowPtr window) const {
    return max(window->width/OPTIMAL_WIDTH, window->height()/OPTIMAL_HEIGHT)
}

void AutomaticLayout::_dichotomicInsert(ContentWindowPtr window,ContentWindowPtrs& windowVec) const{
    int a = 0;
    int b = windowVec.size -1;
    while(! a == b){
        if(_computeMaxRatio(windowVec[(a + b)/2]) > _computeMaxRatio(window)){
            b = (a + b) / 2;
        }
        else {
            a = (a + b) /2;
        }
    }
    windowVec.insert(windowVec.begin()+a, window);
}


AutomaticLayout::AutomaticLayout(const DisplayGroup& group)
{
    LayoutPolicy(group);
    OPTIMAL_HEIGHT = group.height();
    OPTIMAL_WIDTH = group.width();
}


QRectF LayoutEngine::getFocusedCoord( const ContentWindow& window ) const
{
    return _getFocusedCoord( window, _group.getFocusedWindows( ));
}

//TODO ask if const necessary, would like to add fields that change
void LayoutEngine::updateFocusedCoord( const ContentWindowSet& windows ) const
{
    for( auto& window : windows )
        window->setFocusedCoordinates( _getFocusedCoord( *window, windows ));
}

//TODO is it necessary to redo algorithm everytime this is called ?
QRectF LayoutEngine::_getFocusedCoord(const ContentWindow& window, const ContentWindowSet& windows) const
{
    ContentWindowPtrs windowVec = _sortByMaxRatio(windows);


}

ContentWindowPtrs LayoutEngine::_sortByMaxRatio(const ContentWindowSet& windows) const
{
    std::vector< ContentWindowPtr > windowVec(windows.size());
    for(auto& window : windows)
    {
        _dichotomicInsert(window, windowVec);
    }
    return windowVec;
}
