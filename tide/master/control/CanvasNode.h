#ifndef CANVASNODE_H
#define CANVASNODE_H
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#include "types.h"
#include <boost/enable_shared_from_this.hpp>

class CanvasNode : public QRectF,
                   public boost::enable_shared_from_this<CanvasNode>
{
public:
    using NodePtr = boost::shared_ptr<CanvasNode>;

    CanvasNode(NodePtr rootPtr, NodePtr parent, NodePtr firstChild,
               NodePtr secondChild, QRectF rect);
    CanvasNode(QRectF available_space);
    CanvasNode(NodePtr rootPtr, NodePtr parent, ContentWindowPtr window,
               QRectF rect);
    bool insert(ContentWindowPtr window);
    bool isFree() const;
    bool isRoot() const;
    bool isTerminal() const;
    void updateFocusCoordinates();
    void preview();
    qreal getOccupiedSpace();
    const QRectF AVAILABLE_SPACE = QRectF(0.0, 0.0, 0.0, 0.0);
    NodePtr rootPtr;
    NodePtr parent;
    NodePtr firstChild;
    NodePtr secondChild;

private:
    void _update();
    bool previewed = false;
    void _constrainTerminalIntoRect(const QRectF& rect);
    void _constrainNodeIntoRect(const QRectF& rect);

    QRectF _rectWithoutMargins(const QRectF& rect) const;
    QRectF _rectWithoutMargins(const QRectF& rect,
                               CONTENT_TYPE content_type) const;
    bool _insertRoot(ContentWindowPtr window);
    bool _insertTerminal(ContentWindowPtr window);
    void _computeBoundaries(const QRectF& realSize,
                            QRectF& internalNodeBoundaries,
                            QRectF& internalFreeLeafBoundaries,
                            QRectF& externalFreeLeafBoundaries) const;
    bool _insertSecondChild(ContentWindowPtr window);
    bool _chooseVerticalCut(const QRectF& realSize) const;
    void _setRect(QRectF newRect);
    void _constrainIntoRect(const QRectF& rect);
    ContentWindowPtr content = NULL;
    QRectF _addMargins(ContentWindowPtr window) const;
    QRectF _addMargins(const QRectF& rect) const;
    QRectF _addMargins(const QRectF& rect, CONTENT_TYPE type) const;
    const qreal RESIZE_FACTOR_MAX_FOR_INSERT = 1.3;
};

#endif // CANVASNODE_H
