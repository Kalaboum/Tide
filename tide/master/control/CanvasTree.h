#ifndef CANVASTREE_H
#define CANVASTREE_H
#include "types.h"


class CanvasTree
{
public:
    CanvasTree(ContentWindowPtrs windowVec, QRectF available_space);
    void insert(ContentWindowPtr window);
    QRectF getPosition(int position);
    void updateFocusCoordinates();

private:
    QRectF AVAILABLE_SPACE;
    class CanvasNode;
    typedef boost::shared_ptr<CanvasTree::CanvasNode> NodePtr;
    NodePtr rootNode;
    static QRectF _addMargins(ContentWindowPtr window);

};

#endif // CANVASTREE_H
