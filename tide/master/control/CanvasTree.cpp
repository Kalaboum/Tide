#include "CanvasTree.h"
#include "LayoutPolicy.h"
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#include "types.h"
#include <boost/enable_shared_from_this.hpp>

class CanvasTree::CanvasNode : public QRectF,
                               public boost::enable_shared_from_this<CanvasNode>
{
public:
    CanvasNode(NodePtr rootPtr, NodePtr parent, NodePtr firstChild,
               NodePtr secondChild, QRectF rect);
    CanvasNode(QRectF available_space);
    CanvasNode(NodePtr rootPtr, NodePtr parent, ContentWindowPtr window,
               QRectF rect);
    bool insert(ContentWindowPtr window);
    bool isFree();
    bool isRoot();
    bool isTerminal();
    void updateFocusCoordinates();
    const QRectF AVAILABLE_SPACE = QRectF(0.0, 0.0, 0.0, 0.0);
    NodePtr rootPtr;
    NodePtr parent;
    NodePtr firstChild;
    NodePtr secondChild;

private:
    void _constrainIntoRect(const QRectF& rect);
    void _constrainTerminalIntoRect(const QRectF& rect);
    void _constrainNodeIntoRect(const QRectF& rect);
    QRectF _rectWithoutMargins(QRectF rect, CONTENT_TYPE content_type);
    bool _insertRoot(ContentWindowPtr window);
    bool _insertTerminal(ContentWindowPtr window);
    void _computeBoundaries(const QRectF& realSize,
                            QRectF& internalNodeBoundaries,
                            QRectF& internalFreeLeafBoundaries,
                            QRectF& externalFreeLeafBoundaries);
    void _insertSecondChild(ContentWindowPtr window);
    bool _chooseVerticalCut(QRectF realSize);
    void _setRect(QRectF newRect);
    ContentWindowPtr content = NULL;
};

CanvasTree::CanvasTree(ContentWindowPtrs windowVec, QRectF available_space)
{
    rootNode = boost::make_shared<CanvasNode>(CanvasNode(available_space));
    rootNode->rootPtr = rootNode;
    for (size_t i = 0; i < windowVec.size(); i++)
    {
        rootNode->insert(windowVec[i]);
    }
}

QRectF CanvasTree::_addMargins(const ContentWindowPtr window)
{
    QRectF rectWithMargins =
        QRectF(window->x(), window->y(), window->width(), window->height());
    rectWithMargins.setTop(rectWithMargins.top() -
                           controlSpecifications::WINDOW_SPACING_PX -
                           controlSpecifications::WINDOW_TITLE_HEIGHT);
    rectWithMargins.setLeft(rectWithMargins.left() -
                            controlSpecifications::WINDOW_CONTROLS_MARGIN_PX -
                            controlSpecifications::WINDOW_SPACING_PX);
    if (window->getContentPtr()->getType() == CONTENT_TYPE_MOVIE)
    {
        rectWithMargins.setTop(rectWithMargins.top() -
                               controlSpecifications::MOVIE_BAR_HEIGHT);
    }
    return rectWithMargins;
}

void CanvasTree::updateFocusCoordinates()
{
    rootNode->updateFocusCoordinates();
}

CanvasTree::CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent,
                                   NodePtr _firstChild, NodePtr _secondChild,
                                   QRectF rect)
    : rootPtr(_rootPtr)
    , parent(_parent)
    , firstChild(_firstChild)
    , secondChild(_secondChild)
{
    _setRect(rect);
}

CanvasTree::CanvasNode::CanvasNode(QRectF available_space)
    : AVAILABLE_SPACE(available_space)
{
}

CanvasTree::CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent,
                                   ContentWindowPtr window, QRectF rect)
    : rootPtr(_rootPtr)
    , parent(_parent)
    , content(window)
{
    _setRect(rect);
}

bool CanvasTree::CanvasNode::insert(ContentWindowPtr window)
{
    if (isRoot())
    {
        _insertRoot(window);
    }
    else if (isTerminal())
    {
        return _insertTerminal(window);
    }
    else
    {
        if (firstChild->insert(window))
            return true;
        else if (secondChild->insert(window))
            return true;
    }
    return false;
}

bool CanvasTree::CanvasNode::isFree()
{
    if (isTerminal())
        return content == nullptr;
    else
        return firstChild->isFree() && secondChild->isFree();
}

bool CanvasTree::CanvasNode::isRoot()
{
    return parent == nullptr;
}

bool CanvasTree::CanvasNode::isTerminal()
{
    return (firstChild && secondChild);
}

void CanvasTree::CanvasNode::updateFocusCoordinates()
{
    // available space is not entirety of the display group
    _constrainIntoRect(AVAILABLE_SPACE);
}

QRectF CanvasTree::CanvasNode::_rectWithoutMargins(QRectF rect,
                                                   CONTENT_TYPE content_type)
{
    // take care that margins are respected
    QRectF rectWithoutMargins =
        QRectF(rect.left() + controlSpecifications::WINDOW_CONTROLS_MARGIN_PX,
               rect.top() + controlSpecifications::WINDOW_TITLE_HEIGHT,
               rect.width() - controlSpecifications::WINDOW_CONTROLS_MARGIN_PX -
                   controlSpecifications::WINDOW_SPACING_PX,
               rect.height() - controlSpecifications::WINDOW_SPACING_PX -
                   controlSpecifications::WINDOW_TITLE_HEIGHT);
    if (content_type == CONTENT_TYPE_MOVIE)
    {
        rectWithoutMargins.setTop(rectWithoutMargins.top() +
                                  controlSpecifications::MOVIE_BAR_HEIGHT);
    }
    return rectWithoutMargins;
}

void CanvasTree::CanvasNode::_constrainTerminalIntoRect(const QRectF& rect)
{
    QRectF rectWithoutMargins =
        _rectWithoutMargins(rect, content->getContentPtr()->getType());
    qreal scaleFactor =
        std::min(rectWithoutMargins.width() / content->width(),
                 rectWithoutMargins.height() / content->height());
    qreal newWidth = content->width() * scaleFactor;
    qreal newHeight = content->height() * scaleFactor;
    qreal newLeft =
        rectWithoutMargins.left() + (rectWithoutMargins.width() - newWidth) / 2;
    qreal newTop = rectWithoutMargins.top() +
                   (rectWithoutMargins.height() - newHeight) / 2;
    QRectF newRect = QRectF(newLeft, newTop, newWidth, newHeight);
    content->setFocusedCoordinates(newRect);
    QRectF rectWithMargins = _addMargins(content);
    setRect(rectWithMargins.left(), rectWithMargins.top(),
            rectWithMargins.width(), rectWithMargins.height());
}

void CanvasTree::CanvasNode::_constrainNodeIntoRect(const QRectF& rect)
{
    qreal scaleFactor =
        std::min(rect.width() / width(), rect.height() / height());
    qreal newWidth = width() * scaleFactor;
    qreal newHeight = height() * scaleFactor;
    qreal newLeft = rect.left() + (rect.width() - newWidth) / 2;
    qreal newTop = rect.top() + (rect.height() - newHeight) / 2;
    qreal firstChildNewWidth = newWidth * firstChild->width() / width();
    qreal firstChildNewHeight = newHeight * firstChild->height() / height();
    firstChild->_constrainIntoRect(
        QRectF(newLeft, newTop, firstChildNewWidth, firstChildNewHeight));
    // TODO see if this causes issues
    if (secondChild->top() == top())
    {
        secondChild->_constrainIntoRect(
            QRectF(newLeft + firstChildNewWidth, newTop,
                   newWidth - firstChildNewWidth, firstChildNewHeight));
    }
    else
    {
        secondChild->_constrainIntoRect(
            QRectF(newLeft, newTop + firstChildNewHeight, firstChildNewWidth,
                   newHeight - firstChildNewHeight));
    }
    setRect(newLeft, newTop, newWidth, newHeight);
}

void CanvasTree::CanvasNode::_constrainIntoRect(const QRectF& rect)
{
    if (isRoot() && secondChild)
    {
        if (firstChild)
        {
            return;
        }
        firstChild->_constrainIntoRect(rect);
        return;
    }
    else if (isTerminal())
    {
        _constrainTerminalIntoRect(rect);
    }
    else if (secondChild->isFree())
    {
        firstChild->_constrainIntoRect(rect);
    }
    else
    {
        _constrainNodeIntoRect(rect);
    }
}
bool CanvasTree::CanvasNode::_insertRoot(ContentWindowPtr window)
{
    if (firstChild != NULL)
    {
        if (firstChild->insert(window))
            return true;
        else if (secondChild != NULL)
        {
            if (secondChild->insert(window))
                return true;
            else
            { // we have to create some new space
                NodePtr newNodePtr = boost::make_shared<CanvasNode>(
                    CanvasNode(rootPtr, rootPtr, firstChild, secondChild,
                               QRectF(topLeft(), size())));
                firstChild = newNodePtr;
                _insertSecondChild(window);
            }
        }
        else
        {
            _insertSecondChild(window);
        }
    }
    else
    {
        firstChild = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, rootPtr, window, _addMargins(window)));
        _setRect(_addMargins(window));
    }
    return true;
}

void CanvasTree::CanvasNode::_computeBoundaries(
    const QRectF& realSize, QRectF& internalNodeBoundaries,
    QRectF& internalFreeLeafBoundaries, QRectF& externalFreeLeafBoundaries)
{
    if (realSize.width() / width() > realSize.height() / height())
    { // horizontal cut
        internalNodeBoundaries.setRect(left(), top(), width(),
                                       realSize.height());
        internalFreeLeafBoundaries.setRect(left() + realSize.width(), top(),
                                           width() - realSize.width(),
                                           realSize.height());
        externalFreeLeafBoundaries.setRect(left(), top() + realSize.height(),
                                           width(),
                                           height() - realSize.height());
    } // vertical cut
    else
    {
        internalNodeBoundaries.setRect(left(), top(), realSize.width(),
                                       height());
        internalFreeLeafBoundaries.setRect(left(), top() + realSize.height(),
                                           realSize.width(),
                                           height() - realSize.height());
        externalFreeLeafBoundaries.setRect(left() + realSize.height(), top(),
                                           width() - realSize.width(),
                                           height());
    }
}

bool CanvasTree::CanvasNode::_insertTerminal(ContentWindowPtr window)
{
    if (!isFree())
    {
        return false;
    }
    QRectF realSize = _addMargins(window);
    if (realSize.width() <= width() && realSize.height() <= height())
    {
        // separate depending on ratio (vertical or horizontal cut
        QRectF internalNodeBoundaries;
        QRectF internalFreeLeafBoundaries;
        QRectF externalFreeleafBoundaries;
        _computeBoundaries(realSize, internalNodeBoundaries,
                           internalFreeLeafBoundaries,
                           externalFreeleafBoundaries);

        NodePtr thisPtr = shared_from_this();
        NodePtr internalNodePtr = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, thisPtr, NULL, NULL, internalNodeBoundaries));
        NodePtr firstChildPtr = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, internalNodePtr, window,
                       QRectF(left(), top(), realSize.width(),
                              realSize.height())));
        NodePtr secondChildPtr = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, internalNodePtr, NULL,
                       internalFreeLeafBoundaries));
        internalNodePtr->firstChild = firstChildPtr;
        internalNodePtr->secondChild = secondChildPtr;
        NodePtr externalFreeLeafPtr = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, thisPtr, NULL, externalFreeleafBoundaries));
        firstChild = internalNodePtr;
        secondChild = externalFreeLeafPtr;
        return true;
    }
    return false;
}

// This method is called only by the rootNode : it creates some space
void CanvasTree::CanvasNode::_insertSecondChild(ContentWindowPtr window)
{
    QRectF realSize = _addMargins(window);
    if (_chooseVerticalCut(realSize))
    {
        if (realSize.height() > height())
        { // meaning we would have a to add some space
            NodePtr newEmptySpace = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, NULL, NULL,
                           QRectF(left(), height(), width(),
                                  realSize.height() - height())));
            NodePtr newFirstChildNode = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, rootPtr, firstChild, newEmptySpace,
                           QRectF(left(), top(), width(), realSize.height())));
            firstChild = newFirstChildNode;
            newFirstChildNode->firstChild->parent = newFirstChildNode;
            secondChild = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, rootPtr, NULL,
                           QRectF(width(), top(), realSize.width(),
                                  realSize.height())));
            setRect(left(), top(), width() + realSize.width(),
                    realSize.height());
        }
        else
        {
            secondChild = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, rootPtr, NULL,
                           QRectF(width(), top(), realSize.width(), height())));
            setWidth(width() + realSize.width());
        }
    }
    else
    {
        if (realSize.width() > width())
        {
            NodePtr newEmptySpace = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, NULL, NULL,
                           QRectF(width(), top(), realSize.width() - width(),
                                  height())));
            NodePtr newFirstChildNode = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, rootPtr, firstChild, newEmptySpace,
                           QRectF(left(), top(), realSize.width(), height())));
            firstChild = newFirstChildNode;
            newFirstChildNode->firstChild->parent = newFirstChildNode;
            secondChild = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, rootPtr, NULL,
                           QRectF(left(), height(), realSize.width(),
                                  realSize.height())));
            setRect(left(), top(), realSize.width(),
                    height() + realSize.height());
        }
        else
        {
            secondChild = boost::make_shared<CanvasNode>(
                CanvasNode(rootPtr, rootPtr, NULL,
                           QRectF(left(), height(), width(),
                                  realSize.height())));
            setHeight(height() + realSize.height());
        }
    }
}
bool CanvasTree::CanvasNode::_chooseVerticalCut(QRectF realSize)
{
    return (width() + realSize.width()) / AVAILABLE_SPACE.width() <
           (height() + realSize.height()) / AVAILABLE_SPACE.height();
}

void CanvasTree::CanvasNode::_setRect(QRectF newRect)
{
    setRect(newRect.left(), newRect.top(), newRect.width(), newRect.height());
}
