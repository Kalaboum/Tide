#include "CanvasTree.h"
#include "types.h"
//TODO fix that
#include "../../core/scene/Rectangle.h"
typedef boost::shared_ptr<CanvasComponent> ComponentPtr;
//TODO  check the specs
struct WindowControlSpecification{
    const qreal WIDTH_MARGIN;
    const qreal WIDTH;
    const qreal HEIGHT;
};

CanvasTree::CanvasTree(ContentWindowPtrs windowVec)
{
    rootNode = CanvasNode(NULL, NULL, NULL, QRectF(0.0, 0.0, 0.0, 0.0))
    for(auto& window : windowVec){
            rootNode.insert(window);
    }
}

static CanvasTree::QRectF _addMargins(ContentWindowPtr window){
    return QRectF(window->left, window->top,
                  windowCtrlSpecs.WIDTH + windowCtrlSpecs.WIDTH_MARGIN * 2 + window->width,
                  max(window->height, windowCtrlSpecs.HEIGHT))
}


class CanvasTree::CanvasComponent : public QRectF{
    public:
        virtual bool insert(ContentWindowPtr window) = 0;
        ComponentPtr getParent(){
            return parent;
        }
        ComponentPtr getFirstChild(){
            return firstChild;
        }
        ComponentPtr getSecondChild(){
            return secondChild;
        }
        void setParent(ComponentPtr parent){
            this->parent = parent;
        }

        virtual void setFirstChild(ComponentPtr firstChild);
        virtual void setSecondChild(ComponentPtr secondChild);

    private:
        ComponentPtr parent;
        ComponentPtr firstChild;
        ComponentPtr secondChild;
};

class CanvasTree::CanvasRoot : public CanvasComponent{
    //TODO maybe inherit from CanvasNode
    public:
        CanvasRoot(){}
        bool insert(ContentWindowPtr window){
            if(firstChild!=NULL){
                if(firstChild->insert(window)) return true;
                else if(secondChild != NULL){
                    if(secondChild.insert(window)) return true;
                    else{//we have to create some new space
                        ComponentPtr newNodePtr = boost::shared_ptr(CanvasNode(rootPtr, firstChild, secondChild, this->getCoords()));
                        firstChild = newNodePtr;
                        _insertSecondChild(window);
                    }
                }
                else {
                    _insertSecondChild(window);
                }
            }
            else{
               firstChild = boost::shared_ptr(CanvasLeaf(rootPtr, window, _addMargins(window)));
               this->setCoords(_addMargins(window));
            }
            return true;
        }
     private:
        void _insertSecondChild(ContentWindowPtr window){
            if(_chooseVerticalCut(realSize)){
                QRectF realSize = _addMargins(window);
                secondChild = boost::shared_ptr(CanvasLeaf(rootPtr, NULL, QRectF(this->width(), this->top(), realSize.width(), this->height())));
                secondChild.insert(window);
                this->setWidth(this->width() + realSize.width());
            }else {
                secondChild = boost::shared_ptr(CanvasLeaf(rootPtr, NULL, QRectF(this->left(), this->height(), this->width(), realSize.height())));
                secondChild.insert(window);
                this->setHeight(this->height() + realSize.height());
            }
        }
        bool _chooseVerticalCut(QRectF realSize){
            (this->width() + realSize.width() / OPTIMAL_WIDTH) < (this->height() + realSize.height() / OPTIMAL_HEIGHT);
        }
        ComponentPtr rootPtr = boost::shared_ptr(this);
        const qreal OPTIMAL_WIDTH;
        const qreal OPTIMAL_HEIGHT;
}

class CanvasTree::CanvasNode : public CanvasComponent{
    public:
        CanvasNode(ComponentPtr parent, ComponentPtr firstChild, ComponentPtr secondChild, QRectF coordinates)
            :parent(parent), firstChild(firstChild), secondChild(secondChild)
        {
            this->setCoords(coordinates);
        }
        bool insert(ContentWindowPtr window):{
            if(firstChild->insert(window)) return true;
            else if (secondChild->insert(window)) return true;
            return false;
        }
        void setFirstChild(ComponentPtr firstChild){
            this->firstChild = firstChild;
        }

        void setSecondChild(ComponentPtr secondChild){
            this->secondChild = secondChild;
        }

    private:

};

class CanvasTree::CanvasLeaf : public CanvasComponent{
    public:
        CanvasLeaf(ComponentPtr parent, ContentWindowPtr window, QRectF coordinates)
            : parent(parent), window(window){
            firstChild = NULL;
            secondChild = NULL;
            this->setCoords(coordinates)
        }
        bool insert(ContentWindowPtr window){
            if(!isFree()){
                return false;
            }
            QRectF realSize = _addMargins(window);
            if(realSize.width <= this->width() && realSize.height <= this->height()){
                //separate depending on ratio (vertical or horizontal cut
                QRect externalNodeBoundaries = QRectF(this->left(), this->top(), this->width(), this->height());
                QRect internalNodeBoundaries;
                QRect internalFreeLeafBoundaries;
                QRect externalFreeleafBoundaries;
                if(realSize.width()/ this->width() > realSize.height()/ this->height()){
                    internalNodeBoundaries = QRectF(this->left(), this->top(), this->width(), realSize.height());
                    internalFreeLeafBoundaries = QRectF(this->left() + realSize.width(), this->top(),
                                                       this->width - realSize.width(), realSize.height());
                    externalFreeleafBoundaries; QRectF(this->left(), this->top() + realSize.height(),
                                                       this->width(), this->height() - realSize.height());
                }
                else{
                    internalNodeBoundaries = QRectF(this->left(), this->top(), realSize.width(), this->height());
                    internalFreeLeafBoundaries = QRectF(this->left(), this->top() + realSize.height(),
                                                       this->width, realSize.height() - realSize.height());
                    externalFreeleafBoundaries; QRectF(this->left() + realSize.height(), this->top(),
                                                       this->width()- realSize.width(), this->height());
                }
                this->setCoords(this->left(), this->top(), realSize.width(), realSize.height());
                this->window = window;
                CanvasNode externalNode = CanvasNode(parent, NULL, NULL, externalNodeBoundaries);
                ComponentPtr externalNodePtr = boost::shared_ptr(externalNode);
                CanvasNode internalNode = CanvasNode(externalNodePtr, boost::shared_ptr(this), NULL,
                                                     internalNodeBoundaries);
                ComponentPtr internalNodePtr = boost::shared_ptr(internalNode);
                CanvasLeaf internalFreeLeaf = CanvasLeaf(internalNodePtr, NULL, internalFreeLeafBoundaries);
                internalNode.setSecondChild(boost::shared_ptr(internalFreeLeaf));
                externalNode.setFirstChild(boost::shared_ptr(internalNode));
                CanvasLeaf externalFreeLeaf = CanvasLeaf(externalNodePtr, NULL, externalFreeleafBoundaries);
                externalNode.setSecondChild(boost::shared_ptr(externalFreeLeaf));
                externalNode.setParent(this->parent);
                this->setParent(internalNodePtr);
                return true;
            }
            return false;
        }
        bool isFree(){
            return window == NULL;
        }

        void setFirstChild(ComponentPtr firstChild){
        }

        void setSecondChild(ComponentPtr secondChild){
        }

    private:
        ComponentPtr parent;
        ContentWindowPtr window;

};

