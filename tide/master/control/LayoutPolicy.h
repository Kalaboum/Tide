#ifndef LAYOUTPOLICY_H
#define LAYOUTPOLICY_H
#include "types.h"

namespace controlSpecifications{
const qreal INSIDE_MARGIN_RELATIVE = 0.02;
const qreal SIDEBAR_WIDTH_REL_TO_DISPLAYGROUP_HEIGHT = 0.3 * 0.3;
const qreal WINDOW_CONTROLS_MARGIN_PX = 200.0;
const qreal WINDOW_SPACING_PX = 80.0;
const qreal WINDOW_TITLE_HEIGHT = 100.0;
}
class LayoutPolicy
{
public:
    LayoutPolicy( const DisplayGroup& group );

    /** @return the focused coordinates for the window. */
    virtual QRectF getFocusedCoord( const ContentWindow& window ) const=0;

    /** Update the focused coordinates for the set of windows. */
    virtual void updateFocusedCoord( const ContentWindowSet& windows ) const=0;
    virtual ~LayoutPolicy(){};

protected:
    const DisplayGroup& _group;
    QRectF _getAvailableSpace() const;

};

#endif
