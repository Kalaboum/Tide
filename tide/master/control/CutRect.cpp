#include "CutRect.h"
#include "LayoutPolicy.h"
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
    // Can have issue with floats, tolerance is added
    bool tooLeft = rect.left() + rect.width() <= _firstWidthCut->getX() + 1;
    bool tooRight = rect.left() >= _secondWidthCut->getX() - 1;
    bool tooHigh = rect.top() + rect.width() <= _firstHeightCut->getY() + 1;
    bool tooLow = rect.top() >= _secondHeightCut->getY() - 1;
    return (!(tooLeft || tooRight) && (!(tooHigh || tooLow)));
}

QRectF CutRect::getCorrespondingRect() const
{
    return QRectF(_firstWidthCut->getX(), _firstHeightCut->getY(),
                  _secondWidthCut->getX() - _firstWidthCut->getX(),
                  _secondHeightCut->getY() - _firstHeightCut->getY());
}

ContentWindowPtr CutRect::getWindow()
{
    return _window;
}

int CutRect::beginOrderHeight()
{
    return _firstHeightCut->getOrder();
}

int CutRect::endOrderHeight()
{
    return _secondHeightCut->getOrder();
}

int CutRect::beginOrderWidth()
{
    return _firstWidthCut->getOrder();
}

int CutRect::endOrderWidth()
{
    return _secondWidthCut->getOrder();
}

void CutRect::changeCuts(CutPtr firstWidthCut, CutPtr secondWidthCut,
                         CutPtr firstHeightCut, CutPtr secondHeightCut)
{
    _firstWidthCut = firstWidthCut;
    _secondWidthCut = secondWidthCut;
    _firstHeightCut = firstHeightCut;
    _secondHeightCut = secondHeightCut;
}

void CutRect::updateWindowSize()
{
    QRectF rectWithoutMargins =
        LayoutPolicy::rectWithoutMargins(getCorrespondingRect(),
                                         _window->getContentType());
    _window->setCoordinates(rectWithoutMargins);
}
