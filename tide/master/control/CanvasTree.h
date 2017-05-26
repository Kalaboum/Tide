#ifndef CANVASTREE_H
#define CANVASTREE_H
#include "CanvasNode.h"
#include "types.h"

class CanvasTree
{
public:
    CanvasTree(ContentWindowPtrs windowVec, QRectF available_space);
    void insert(ContentWindowPtr window);
    QRectF getPosition(int position);
    void updateFocusCoordinates();
    qreal getOccupiedSpace();

private:
    using NodePtr = boost::shared_ptr<CanvasNode>;
    NodePtr rootNode;
};

#endif // CANVASTREE_H
