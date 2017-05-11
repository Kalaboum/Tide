#ifndef CUTALGO_H
#define CUTALGO_H
#include "CutGrid.h"
#include "LayoutPolicy.h"
#include "types.h"

class CutAlgo : public LayoutPolicy
{
    using LayoutPolicy::LayoutPolicy;

public:
    CutAlgo(const DisplayGroup& group);
    /** @return the focused coordinates for the window. */
    QRectF getFocusedCoord(const ContentWindow& window) const;

    /** Update the focused coordinates for the set of windows. */
    void updateFocusedCoord(const ContentWindowSet& windows) const;

private:
    bool _isInRectangle(CutPointPtr point, QRectF enclosingRectangle,
                        ContentWindowPtr window);
    CutGrid grid;
};

#endif // CUTALGO_H
