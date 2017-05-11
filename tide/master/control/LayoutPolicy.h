#ifndef LAYOUTPOLICY_H
#define LAYOUTPOLICY_H
#include "types.h"

// Hardcoded variables which are also defined in style.js
namespace controlSpecifications
{
const qreal INSIDE_MARGIN_RELATIVE = 0.02;
const qreal SIDEBAR_WIDTH_REL_TO_DISPLAYGROUP_HEIGHT = 0.3 * 0.3;
const qreal WINDOW_CONTROLS_MARGIN_PX = 200.0;
const qreal WINDOW_SPACING_PX = 80.0;
const qreal WINDOW_TITLE_HEIGHT = 100.0;
// TODO check this, could not see it in style.js
const qreal MOVIE_BAR_HEIGHT = 100.0;
}

/**
 * Abstract class whose derived classes must deal with the layout in focus
 * coordinates
 * @brief The LayoutPolicy class
 */
class LayoutPolicy
{
public:
    LayoutPolicy(const DisplayGroup& group);

    /** @return the focused coordinates for the window. */
    virtual QRectF getFocusedCoord(const ContentWindow& window) const = 0;

    /** Update the focused coordinates for the set of windows. */
    virtual void updateFocusedCoord(const ContentWindowSet& windows) const = 0;
    virtual ~LayoutPolicy(){};
    static QRectF _addMargins(const ContentWindowPtr window);

protected:
    const DisplayGroup& _group;
    QRectF _getAvailableSpace() const;
    qreal _computeMaxRatio(ContentWindowPtr) const;
    ContentWindowPtrs _sortByMaxRatio(const ContentWindowSet& windows) const;
};

#endif
