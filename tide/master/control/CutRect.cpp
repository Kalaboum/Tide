#include "CutRect.h"

CutRect CutRect::createCutRect(CutPtr firstWidthCut, CutPtr secondWidthCut,
                               CutPtr firstHeightCut, CutPtr secondHeightCut,
                               ContentWindowPtr window)
{
    assert(firstWidthCut->getX() < secondWidthCut->getX());
    assert(firstHeightCut->getY() < secondHeightCut->getY());
    return CutRect(firstWidthCut, secondWidthCut, firstHeightCut,
                   secondHeightCut, window);
}
CutRect::CutRect(CutPtr firstWidthCut, CutPtr secondWidthCut,
                 CutPtr firstHeightCut, CutPtr secondHeightCut,
                 ContentWindowPtr window)
    : _firstWidthCut(firstWidthCut)
    , _secondWidthCut(secondWidthCut)
    , _firstHeightCut(firstHeightCut)
    , _secondHeightCut(secondHeightCut)
    , _window(window)
{
}

bool CutRect::intersectWith(const QRectF& rect) const
{
    bool tooLeft = rect.left() + rect.width() <= _firstWidthCut->getX();
    bool tooRight = rect.left() >= _secondWidthCut->getX();
    bool tooHigh = rect.top() + rect.width() <= _firstHeightCut->getY();
    bool tooLow = rect.top() >= _secondHeightCut->getY();
    return (!(tooLeft || tooRight) && (!(tooHigh || tooLow)));
}
