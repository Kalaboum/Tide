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
    static CutRect createCutRect(CutPtr firstWidthCut, CutPtr secondWidthCut,
                                 CutPtr firstHeightCut, CutPtr secondHeightCut,
                                 ContentWindowPtr window);

    static QRectF getScaledRect(const QRectF& rectToScale,
                                const QRectF& bounds);
    bool intersectWith(const QRectF& rect) const;
    QRectF getCorrespondingRect() const;
    ContentWindowPtr getWindow();
    size_t beginOrderWidth() const;
    size_t endOrderWidth() const;
    size_t beginOrderHeight() const;
    size_t endOrderHeight() const;
    std::vector<CutPtr> giveNewBounds(CutPtr firstWidthCut,
                                      CutPtr secondWidthCut,
                                      CutPtr firstHeightCut,
                                      CutPtr secondHeightCut);
    void updateWindowSize();

private:
    CutRect(CutPtr firstWidthCut, CutPtr secondWidthCut, CutPtr firstHeightCut,
            CutPtr secondHeightCut, ContentWindowPtr window);
    CutPtr _firstWidthCut;
    CutPtr _secondWidthCut;
    CutPtr _firstHeightCut;
    CutPtr _secondHeightCut;
    ContentWindowPtr _window;
};

typedef boost::shared_ptr<CutRect> CutRectPtr;

#endif // CUTRECT_H
