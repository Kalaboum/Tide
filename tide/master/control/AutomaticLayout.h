#ifndef AUTOMATICLAYOUT_H
#define AUTOMATICLAYOUT_H
#include "types.h"
#include "LayoutPolicy.h"



class AutomaticLayout : public LayoutPolicy
{
    using LayoutPolicy::LayoutPolicy;
public:
    AutomaticLayout(const DisplayGroup& group);
    AutomaticLayout(const DisplayGroup &group, bool separateMovies);

    /** @return the focused coordinates for the window. */
    QRectF getFocusedCoord( const ContentWindow& window ) const;

    /** Update the focused coordinates for the set of windows. */
    void updateFocusedCoord( const ContentWindowSet& windows ) const;

private:
    std::vector<ContentWindowSet> _separateContent(const ContentWindowSet& windows) const;
    qreal _getTotalArea(const ContentWindowSet& windows) const;
    qreal _computeMaxRatio(ContentWindowPtr) const;
    void _dichotomicInsert(ContentWindowPtr, ContentWindowPtrs&) const;
    QRectF _getFocusedCoord(const ContentWindow& window, const ContentWindowSet& windows) const;
    ContentWindowPtrs _sortByMaxRatio(const ContentWindowSet& windows) const;
    bool _separateMovies = true;
};

#endif // AUTOMATICLAYOUT_H
