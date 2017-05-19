#include "Cut.h"

Cut::Cut(qreal x, qreal y)
    : X(x)
    , Y(y)
{
}

Cut Cut::widthCut(qreal x)
{
    return Cut(x, -1.0);
}

Cut Cut::heightCut(qreal y)
{
    return Cut(-1.0, y);
}

qreal Cut::getX() const
{
    return X;
}

qreal Cut::getY() const
{
    return Y;
}

void Cut::scale(qreal scaleFactor)
{
    if (Y > 0)
    {
        Y *= scaleFactor;
    }
    else if (X > 0)
    {
        X *= scaleFactor;
    }
}

void Cut::setOrder(int order)
{
    _order = order;
}
