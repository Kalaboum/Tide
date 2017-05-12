#include "CutPoint.h"
CutPoint::CutPoint(CutPtr widthCut, CutPtr heightCut)
{
    assert(widthCut->getX() >= 0.0);
    assert(heightCut->getY() >= 0.0);
    wCut = widthCut;
    hCut = heightCut;
}

const CutPtr CutPoint::getHeightCut() const
{
    return hCut;
}

const CutPtr CutPoint::getWidthCut() const
{
    return wCut;
}

qreal CutPoint::getX() const
{
    return wCut->getX();
}

qreal CutPoint::getY() const
{
    return hCut->getY();
}
