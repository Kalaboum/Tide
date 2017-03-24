#include "CanvasTree.h"
#include "types.h"
#include "LayoutPolicy.h"
#include "scene/ContentWindow.h"
#include <boost/enable_shared_from_this.hpp>
//TODO  margin for the title
class CanvasTree::CanvasNode : public QRectF, public boost::enable_shared_from_this<CanvasNode>{
public:
    CanvasNode(NodePtr rootPtr, NodePtr parent, NodePtr firstChild, NodePtr secondChild, QRectF rect);
    CanvasNode(QRectF available_space);
    CanvasNode(NodePtr rootPtr, NodePtr parent, ContentWindowPtr window, QRectF rect);
    bool insert(ContentWindowPtr window);
    bool isFree();
    bool isRoot();
    bool isTerminal();
    void updateFocusCoordinates();
    const QRectF AVAILABLE_SPACE = QRectF(0.0, 0.0, 0.0, 0.0);
    NodePtr rootPtr;
    NodePtr parent = NULL;
    NodePtr firstChild = NULL;
    NodePtr secondChild = NULL;
private:
    void _constrainIntoRect(QRectF rect);
    QRectF _rectWithoutMargins(QRectF rect);
    bool _insertRoot(ContentWindowPtr window);
    bool _insertTerminal(ContentWindowPtr window);
    void _insertSecondChild(ContentWindowPtr window);
    bool _chooseVerticalCut(QRectF realSize);
    void _setRect(QRectF newRect);
    ContentWindowPtr content = NULL;
};

CanvasTree::CanvasTree(ContentWindowPtrs windowVec, QRectF available_space)
{
    rootNode = boost::make_shared<CanvasNode>(CanvasNode(available_space));
    rootNode->rootPtr = rootNode;
    for(size_t i = 0 ; i < windowVec.size(); i++) {
        rootNode->insert(windowVec[i]);
    }
}

QRectF CanvasTree::_addMargins(ContentWindowPtr window){
    return QRectF(window->x(), window->y(),
                  controlSpecifications::WINDOW_CONTROLS_MARGIN_PX + controlSpecifications::WINDOW_SPACING_PX + window->width(),
                  window->height() + controlSpecifications::WINDOW_SPACING_PX);
}

void CanvasTree::updateFocusCoordinates(){
    rootNode->updateFocusCoordinates();
}


CanvasTree::CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent, NodePtr _firstChild, NodePtr _secondChild, QRectF rect):
    rootPtr(_rootPtr), parent(_parent), firstChild(_firstChild), secondChild(_secondChild)
{
    _setRect(rect);
}

CanvasTree::CanvasNode::CanvasNode(QRectF available_space):
 AVAILABLE_SPACE(available_space){
}

CanvasTree::CanvasNode::CanvasNode(NodePtr _rootPtr, NodePtr _parent, ContentWindowPtr window, QRectF rect) :
    rootPtr(_rootPtr), parent(_parent), content(window)
{
    _setRect(rect);
}



bool CanvasTree::CanvasNode::insert(ContentWindowPtr window){
    if(isRoot()){
            _insertRoot(window);
    }
    else if(isTerminal()) {
        _insertTerminal(window);
    }
    else{
        if(firstChild->insert(window)) return true;
        else if (secondChild->insert(window)) return true;
    }
    return false;
}

bool CanvasTree::CanvasNode::isFree(){
    if(isTerminal()) return content == NULL;
    else return firstChild->isFree() && secondChild->isFree();
}

bool CanvasTree::CanvasNode::isRoot(){
    return parent == NULL;
}

bool CanvasTree::CanvasNode::isTerminal(){
    return firstChild == NULL && secondChild == NULL;
}

void CanvasTree::CanvasNode::updateFocusCoordinates(){
    //available space is not entirety of the display group
    this->_constrainIntoRect(AVAILABLE_SPACE);
}

QRectF CanvasTree::CanvasNode::_rectWithoutMargins(QRectF rect){
    //take care that margins are respected
    return QRectF(rect.left() + controlSpecifications::WINDOW_CONTROLS_MARGIN_PX, rect.top(),
                             rect.width() - controlSpecifications::WINDOW_CONTROLS_MARGIN_PX - controlSpecifications::WINDOW_SPACING_PX,
                             rect.height() - controlSpecifications::WINDOW_SPACING_PX);
}

//TODO check min and max size constraints on content window
void CanvasTree::CanvasNode::_constrainIntoRect(QRectF rect){
    if(isRoot() && secondChild == NULL){
        if (firstChild == NULL){
            return;
        }
        firstChild->_constrainIntoRect(rect);
        return;
    }
    else if(isTerminal()) {
        QRectF rectWithoutMargins = _rectWithoutMargins(rect);
        qreal scaleFactor = std::min(rectWithoutMargins.width()/this->content->width(),
                                     rectWithoutMargins.height()/this->content->height());
        qreal newWidth = this->content->width() * scaleFactor;
        qreal newHeight = this->content->height() * scaleFactor;
        qreal newLeft = rectWithoutMargins.left() + (rectWithoutMargins.width() - newWidth)/2;
        qreal newTop = rectWithoutMargins.top() + (rectWithoutMargins.height() - newHeight)/2;
        QRectF newRect = QRectF(newLeft, newTop, newWidth, newHeight);
        content->setFocusedCoordinates(newRect);
        QRectF rectWithMargins = _addMargins(content);
        setRect(rectWithMargins.left(), rectWithMargins.top(), rectWithMargins.width(), rectWithMargins.height());
    }
    else if(secondChild->isFree()){
        firstChild->_constrainIntoRect(rect);
    }
    else{
        qreal scaleFactor = std::min(rect.width()/this->width(), rect.height()/this->height());
        qreal newWidth = this->width() * scaleFactor;
        qreal newHeight = this->height() * scaleFactor;
        qreal newLeft = rect.left() + (rect.width() - newWidth)/2;
        qreal newTop = rect.top() + (rect.height() - newHeight)/2;
        qreal firstChildNewWidth = newWidth * firstChild->width() / this->width();
        qreal firstChildNewHeight = newHeight * firstChild->height() / this->height();
        firstChild->_constrainIntoRect(QRectF(newLeft, newTop, firstChildNewWidth
                                              , firstChildNewHeight));
        //TODO see if this causes issues
        if(secondChild->top() == this->top()){
            secondChild->_constrainIntoRect(QRectF(newLeft + firstChildNewWidth, newTop, newWidth - firstChildNewWidth, firstChildNewHeight));
        }else{
            secondChild->_constrainIntoRect(QRectF(newLeft, newTop + firstChildNewHeight, firstChildNewWidth, newHeight- firstChildNewHeight));
        }
        setRect(newLeft, newTop, newWidth, newHeight);
    }
}
bool CanvasTree::CanvasNode::_insertRoot(ContentWindowPtr window){
    if(firstChild!=NULL){
        if(firstChild->insert(window)) return true;
        else if(secondChild != NULL){
            if(secondChild->insert(window)) return true;
            else{//we have to create some new space
                NodePtr newNodePtr = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, rootPtr, firstChild, secondChild, QRectF(this->topLeft(),this->size())));
                firstChild = newNodePtr;
                _insertSecondChild(window);
            }
        }
        else {
            _insertSecondChild(window);
        }
    }
    else{
       firstChild = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, rootPtr, window, _addMargins(window)));
       this->_setRect(_addMargins(window));
    }
    return true;
}

bool CanvasTree::CanvasNode::_insertTerminal(ContentWindowPtr window){
    if(!isFree()){
        return false;
    }
    QRectF realSize = _addMargins(window);
    if(realSize.width() <= this->width() && realSize.height() <= this->height()){
        //separate depending on ratio (vertical or horizontal cut
        QRectF internalNodeBoundaries;
        QRectF internalFreeLeafBoundaries;
        QRectF externalFreeleafBoundaries;
        if(realSize.width()/ this->width() > realSize.height()/ this->height()){
            internalNodeBoundaries = QRectF(this->left(), this->top(), this->width(), realSize.height());
            internalFreeLeafBoundaries = QRectF(this->left() + realSize.width(), this->top(),
                                                this->width() - realSize.width(), realSize.height());
            externalFreeleafBoundaries = QRectF(this->left(), this->top() + realSize.height(),
                                               this->width(), this->height() - realSize.height());
        }
        else{
            internalNodeBoundaries = QRectF(this->left(), this->top(), realSize.width(), this->height());
            internalFreeLeafBoundaries = QRectF(this->left(), this->top() + realSize.height(),
                                                this->width(), realSize.height() - realSize.height());
            externalFreeleafBoundaries = QRectF(this->left() + realSize.height(), this->top(),
                                               this->width()- realSize.width(), this->height());
        }
        NodePtr thisPtr = shared_from_this();
        NodePtr internalNodePtr = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, thisPtr, NULL, NULL,
                                                               internalNodeBoundaries));
        NodePtr firstChildPtr = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, internalNodePtr, NULL, QRectF(this->left(), this->top(), realSize.width(), realSize.height())));
        NodePtr secondChildPtr = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, internalNodePtr, NULL, internalFreeLeafBoundaries));
        internalNodePtr->firstChild =  firstChildPtr;
        internalNodePtr->secondChild = secondChildPtr;
        NodePtr externalFreeLeafPtr = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, thisPtr, NULL, externalFreeleafBoundaries));
        firstChild = internalNodePtr;
        secondChild = externalFreeLeafPtr;
        return true;
    }
    return false;

}

void CanvasTree::CanvasNode::_insertSecondChild(ContentWindowPtr window){
    QRectF realSize = _addMargins(window);
    if(_chooseVerticalCut(realSize)){
        secondChild = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, rootPtr, window, QRectF(this->width(), this->top(), realSize.width(), this->height())));
        this->setWidth(this->width() + realSize.width());
    }else {
        secondChild = boost::make_shared<CanvasNode>(CanvasNode(rootPtr, rootPtr, window, QRectF(this->left(), this->height(), this->width(), realSize.height())));
        this->setHeight(this->height() + realSize.height());
    }
}
bool CanvasTree::CanvasNode::_chooseVerticalCut(QRectF realSize){
    return (this->width() + realSize.width()) / AVAILABLE_SPACE.width() < (this->height() + realSize.height()) / AVAILABLE_SPACE.height();
}

void CanvasTree::CanvasNode::_setRect(QRectF newRect){
    setRect(newRect.left(), newRect.top(), newRect.width(), newRect.height());
}

