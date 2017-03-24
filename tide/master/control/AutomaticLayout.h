#ifndef AUTOMATICLAYOUT_H
#define AUTOMATICLAYOUT_H
#include "types.h"


class AutomaticLayout : public LayoutPolicy
{
public:
    AutomaticLayout(const DisplayGroup& group);

    /** @return the focused coordinates for the window. */
    QRectF getFocusedCoord( const ContentWindow& window ) const;

    /** Update the focused coordinates for the set of windows. */
    void updateFocusedCoord( const ContentWindowSet& windows ) const;

private:
    const qreal OPTIMAL_WIDTH;
    const qreal OPTIMAL_HEIGHT;
};

#endif // AUTOMATICLAYOUT_H
