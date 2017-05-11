#ifndef CUTGRID_H
#define CUTGRID_H
#include "Cut.h"
#include "CutPoint.h"
#include "CutRect.h"
#include "types.h"
#include <boost/enable_shared_from_this.hpp>

class CutGrid
{
public:
    using CutPtr = boost::shared_ptr<Cut>;
    using CutRectPtr = boost::shared_ptr<CutRect>;
    using CutPointPtr = boost::shared_ptr<CutPoint>;
    CutGrid();
    std::vector<CutPoint> getInsertionPoints() const;
    bool intersectWithPreviousWindows(const QRectF& rect) const;
    bool insertWindowAtPoint(CutPointPtr point, ContentWindowPtr window);

private:
    void _addCut(CutPtr);
    bool _removeInsertionPoint(CutPointPtr point);

    std::vector<CutPtr> heightCuts;
    std::vector<CutPtr> widthCuts;
    std::vector<CutPointPtr> insertionPoints;
    std::vector<CutRectPtr> windowsAdded;
};

#endif // CUTGRID_H
