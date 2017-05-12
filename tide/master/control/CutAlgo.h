#ifndef CUTALGO_H
#define CUTALGO_H
#include "CutGrid.h"
#include "LayoutPolicy.h"
#include "types.h"

class CutAlgo : public LayoutPolicy
{
    using LayoutPolicy::LayoutPolicy;

public:
    using CutPointPtr = boost::shared_ptr<CutPoint>;
    CutAlgo(const DisplayGroup& group);
    /** @return the focused coordinates for the window. */
    QRectF getFocusedCoord(const ContentWindow& window) const;

    /** Update the focused coordinates for the set of windows. */
    void updateFocusedCoord(const ContentWindowSet& windows) const;

private:
    bool _isInRectangle(CutPointPtr point, const QRectF& enclosingRectangle,
                        ContentWindowPtr window) const;
    QRectF _computeRectIfInsertion(CutPointPtr point,
                                   const QRectF& enclosingRectangle,
                                   ContentWindowPtr window) const;
    QRectF _getEnclosingRectangleOfRatio(const QRectF& rectToEnclose,
                                         const QRectF& rectRatio) const;
    QRectF _defineFirstEnclosingRectangle(ContentWindowPtr window) const;
    CutPointPtr _findBestInsertionPoint(std::vector<CutPointPtr>& points,
                                        QRectF enclosingRectangle,
                                        ContentWindowPtr window) const;
};

#endif // CUTALGO_H
