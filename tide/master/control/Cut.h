#ifndef CUT_H
#define CUT_H
#include "types.h"

class Cut
{
public:
    static Cut widthCut(qreal x);
    static Cut heightCut(qreal y);
    qreal getX() const;
    qreal getY() const;

private:
    Cut(qreal x, qreal y);
    const qreal X = -1.0;
    const qreal Y = -1.0;
};

#endif // CUT_H
