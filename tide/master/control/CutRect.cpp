#include "CutRect.h"
#include "LayoutPolicy.h"
#include "scene/ContentWindow.h"
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

bool CutRect::intersectWith(const QRectF &rect) const
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

size_t CutRect::beginOrderHeight() const
{
    return _firstHeightCut->getOrder();
}

size_t CutRect::endOrderHeight() const
{
    return _secondHeightCut->getOrder();
}

size_t CutRect::beginOrderWidth() const
{
    return _firstWidthCut->getOrder();
}

size_t CutRect::endOrderWidth() const
{
    return _secondWidthCut->getOrder();
}

std::vector<CutPtr> CutRect::giveNewBounds(CutPtr firstWidthCut,
                                           CutPtr secondWidthCut,
                                           CutPtr firstHeightCut,
                                           CutPtr secondHeightCut)
{
    QRectF rectRatio = _window->getCoordinates();
    QRectF newBounds = QRectF(firstWidthCut->getX(), firstHeightCut->getY(),
                              secondWidthCut->getX(), secondHeightCut->getY());
    QRectF newBoundsWithoutMargins =
        LayoutPolicy::rectWithoutMargins(newBounds,
                                         _window->getContentPtr()->getType());
    QRectF scaledRect = getScaledRect(rectRatio, newBoundsWithoutMargins);
    QRectF newRectWithMargins =
        LayoutPolicy::rectWithMargins(scaledRect,
                                      _window->getContentPtr()->getType());
    newRectWithMargins.moveCenter(newBounds.center());
    _firstWidthCut =
        boost::make_shared<Cut>(Cut::widthCut(newRectWithMargins.left()));
    _secondWidthCut =
        boost::make_shared<Cut>(Cut::widthCut(newRectWithMargins.right()));
    _firstHeightCut =
        boost::make_shared<Cut>(Cut::heightCut(newRectWithMargins.top()));
    _secondHeightCut =
        boost::make_shared<Cut>(Cut::heightCut(newRectWithMargins.bottom()));
    std::vector<CutPtr> vect;
    vect.push_back(_firstWidthCut);
    vect.push_back(_secondWidthCut);
    vect.push_back(_firstHeightCut);
    vect.push_back(_secondHeightCut);
    return vect;
}

void CutRect::updateWindowSize()
{
    QRectF rectWithoutMargins =
        LayoutPolicy::rectWithoutMargins(getCorrespondingRect(),
                                         _window->getContentPtr()->getType());
    _window->setFocusedCoordinates(rectWithoutMargins);
}

QRectF CutRect::getScaledRect(const QRectF &rectToScale, const QRectF &bounds)
{
    qreal scaleFactor = std::min(bounds.width() / rectToScale.width(),
                                 bounds.height() / rectToScale.height());
    QRectF newRect = QRectF(0.0, 0.0, rectToScale.width() * scaleFactor,
                            rectToScale.height() * scaleFactor);
    return newRect;
}
