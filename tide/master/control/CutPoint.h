#ifndef CUTPOINT_H
#define CUTPOINT_H
#include "Cut.h"
#include "types.h"

class CutPoint
{
public:
    using CutPtr = boost::shared_ptr<Cut>;
    CutPoint(CutPtr widthCut, CutPtr heightCut);

    const CutPtr getWidthCut() const;
    const CutPtr getHeightCut() const;
    qreal getX() const;
    qreal getY() const;

    // TODO ask raphaël if they can change the pointer
private:
    CutPtr wCut;
    CutPtr hCut;
};

#endif // CUTPOINT_H
