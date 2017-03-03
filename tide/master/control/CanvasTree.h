#ifndef CANVASTREE_H
#define CANVASTREE_H
#include "types.h"

struct WindowControlSpecification;

class CanvasTree
{
public:
    CanvasTree(ContentWindowPtrs);
    void insert(ContentWindowPtr);
    QRectF getPosition(int position);

private:
    class CanvasNode;
    class CanvasLeaf;
    class CanvasRoot;
    class CanvasComponent;
    CanvasNode rootNode;
    static const WindowControlSpecification WINDOW_CTRL_SPECS;
    static QRectF _addMargins(ContentWindowPtr window);
};

#endif // CANVASTREE_H
