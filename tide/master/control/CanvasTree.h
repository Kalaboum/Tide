#ifndef CANVASTREE_H
#define CANVASTREE_H
#include "types.h"


class CanvasTree
{
public:
    CanvasTree(ContentWindowPtrs windowVec, qreal optimal_width, qreal optimal_height);
    void insert(ContentWindowPtr window);
    QRectF getPosition(int position);
    void updateFocusCoordinates();

private:
    class CanvasNode;
    typedef boost::shared_ptr<CanvasTree::CanvasNode> NodePtr;
    NodePtr rootNode;
    static QRectF _addMargins(ContentWindowPtr window);

};

#endif // CANVASTREE_H
