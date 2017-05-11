#ifndef CUTRECT_H
#define CUTRECT_H
#include "Cut.h"
#include "types.h"

/**
 * A Rectangle is defined here by the intersection of 4 cuts ( 2 by 2 parallel)
 * @brief The CutRect class
 */
class CutRect
{
public:
    using CutPtr = boost::shared_ptr<Cut>;

    static CutRect createCutRect(CutPtr firstWidthCut, CutPtr secondWidthCut,
                                 CutPtr firstHeightCut, CutPtr secondHeightCut,
                                 ContentWindowPtr window);

    bool intersectWith(const QRectF& rect) const;

private:
    CutRect(CutPtr firstWidthCut, CutPtr secondWidthCut, CutPtr firstHeightCut,
            CutPtr secondHeightCut, ContentWindowPtr window);
    CutPtr _firstWidthCut;
    CutPtr _secondWidthCut;
    CutPtr _firstHeightCut;
    CutPtr _secondHeightCut;
    ContentWindowPtr _window;
};

#endif // CUTRECT_H
