#ifndef CUTPOINT_H
#define CUTPOINT_H
#include "Cut.h"
#include "types.h"
#include "typesCut.h"

class CutPoint
{
public:
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

typedef boost::shared_ptr<CutPoint> CutPointPtr;
#endif // CUTPOINT_H
