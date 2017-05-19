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
    void scale(qreal scaleFactor, qreal offset);
    void setOrder(size_t order);
    size_t getOrder();

private:
    Cut(qreal x, qreal y);
    qreal X = -1.0;
    qreal Y = -1.0;
    size_t _order;
};

typedef boost::shared_ptr<Cut> CutPtr;

#endif // CUT_H
