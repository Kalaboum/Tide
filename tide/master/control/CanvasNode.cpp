#include "CanvasNode.h"
#include "CanvasTree.h"
#include "LayoutPolicy.h"
#include "scene/ContentType.h"
#include "scene/ContentWindow.h"
#include "types.h"
#include <algorithm>
#include <boost/enable_shared_from_this.hpp>

CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent, NodePtr _firstChild,
                       NodePtr _secondChild, QRectF rect)
    : rootPtr(_rootPtr)
    , parent(_parent)
    , firstChild(_firstChild)
    , secondChild(_secondChild)
{
    _setRect(rect);
}

CanvasNode::CanvasNode(QRectF available_space)
    : AVAILABLE_SPACE(available_space)
{
}

CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent,
                       ContentWindowPtr window, QRectF rect)
    : rootPtr(_rootPtr)
    , parent(_parent)
    , content(window)
{
    _setRect(rect);
}

bool CanvasNode::insert(ContentWindowPtr window)
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

bool CanvasNode::isFree() const
{
    if (isTerminal())
        return content == nullptr;
    else
        return firstChild->isFree() && secondChild->isFree();
}

bool CanvasNode::isRoot() const
{
    return parent == nullptr;
}

bool CanvasNode::isTerminal() const
{
    return (!firstChild && !secondChild);
}
void CanvasNode::updateFocusCoordinates()
{
    _constrainIntoRect(AVAILABLE_SPACE);
}

QRectF CanvasNode::_rectWithoutMargins(const QRectF& rect,
                                       CONTENT_TYPE content_type) const
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

void CanvasNode::_constrainTerminalIntoRect(const QRectF& rect)
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

void CanvasNode::_constrainNodeIntoRect(const QRectF& rect)
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

void CanvasNode::_constrainIntoRect(const QRectF& rect)
{
    if (isRoot() && !secondChild)
    {
        if (!firstChild)
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
bool CanvasNode::_insertRoot(ContentWindowPtr window)
{
    if (firstChild)
    {
        if (firstChild->insert(window))
            return true;
        else if (secondChild)
        {
            if (secondChild->insert(window))
                return true;
            else
            { // we have to create some new space
                NodePtr newNodePtr = boost::make_shared<CanvasNode>(
                    CanvasNode(rootPtr, rootPtr, firstChild, secondChild,
                               QRectF(topLeft(), size())));
                firstChild = newNodePtr;
                return _insertSecondChild(window);
            }
        }
        else
        {
            return _insertSecondChild(window);
        }
    }
    else
    {
        firstChild = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, rootPtr, window, _addMargins(window)));
        _setRect(_addMargins(window));
        return true;
    }
    return false;
}

void CanvasNode::_computeBoundaries(const QRectF& realSize,
                                    QRectF& internalNodeBoundaries,
                                    QRectF& internalFreeLeafBoundaries,
                                    QRectF& externalFreeLeafBoundaries) const
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

bool CanvasNode::_insertTerminal(ContentWindowPtr window)
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
    // adding tolerance, we can resize the window
    else if (realSize.width() <= RESIZE_FACTOR_MAX_FOR_INSERT * width() &&
             realSize.height() <= RESIZE_FACTOR_MAX_FOR_INSERT * height())
    {
        qreal maxRatio =
            std::max(realSize.width() / width(), realSize.height() / height());
        NodePtr thisPtr = shared_from_this();
        qreal newWidthWindow = realSize.width() / maxRatio;
        qreal newHeightWindow = realSize.height() / maxRatio;
        QRectF firstChildBoundaries =
            QRectF(left(), top(), newWidthWindow, newHeightWindow);
        QRectF freeLeafBoundaries =
            QRectF(left() + newWidthWindow, top() + newHeightWindow,
                   width() - newWidthWindow, height() - newHeightWindow);
        firstChild = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, thisPtr, window, firstChildBoundaries));
        secondChild = boost::make_shared<CanvasNode>(
            CanvasNode(rootPtr, thisPtr, NULL, NULL, freeLeafBoundaries));
        return true;
    }
    return false;
}

// This method is called only by the rootNode : it creates some space
bool CanvasNode::_insertSecondChild(ContentWindowPtr window)
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
    return insert(window);
}
bool CanvasNode::_chooseVerticalCut(const QRectF& realSize) const
{
    return (width() + realSize.width()) / AVAILABLE_SPACE.width() <
           (height() + realSize.height()) / AVAILABLE_SPACE.height();
}

void CanvasNode::_setRect(QRectF newRect)
{
    setRect(newRect.left(), newRect.top(), newRect.width(), newRect.height());
}

QRectF CanvasNode::_addMargins(const ContentWindowPtr window)
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
