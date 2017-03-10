#ifndef AUTOMATICLAYOUT_H
#define AUTOMATICLAYOUT_H
#include "types.h"
#include "LayoutPolicy.h"



class AutomaticLayout : public LayoutPolicy
{
    using LayoutPolicy::LayoutPolicy;
public:
    AutomaticLayout(const DisplayGroup& group);

    /** @return the focused coordinates for the window. */
    QRectF getFocusedCoord( const ContentWindow& window ) const;

    /** Update the focused coordinates for the set of windows. */
    void updateFocusedCoord( const ContentWindowSet& windows ) const;

private:
    const qreal OPTIMAL_WIDTH;
    const qreal OPTIMAL_HEIGHT;
    qreal _computeMaxRatio(ContentWindowPtr) const;
    void _dichotomicInsert(ContentWindowPtr, ContentWindowPtrs&) const;
    QRectF _getFocusedCoord(const ContentWindow& window, const ContentWindowSet& windows) const;
    ContentWindowPtrs _sortByMaxRatio(const ContentWindowSet& windows) const;
};

#endif // AUTOMATICLAYOUT_H
