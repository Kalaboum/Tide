#include "CanvasTree.h"
#include "CanvasNode.h"
#include "LayoutPolicy.h"
#include "types.h"

CanvasTree::CanvasTree(ContentWindowPtrs windowVec, QRectF available_space)
{
    rootNode = boost::make_shared<CanvasNode>(CanvasNode(available_space));
    rootNode->rootPtr = rootNode;
    for (size_t i = 0; i < windowVec.size(); i++)
    {
        rootNode->insert(windowVec[i]);
    }
}

void CanvasTree::updateFocusCoordinates()
{
    if (rootNode)
    {
        rootNode->updateFocusCoordinates();
    }
}

qreal CanvasTree::getOccupiedSpace()
{
    if (rootNode)
    {
        return rootNode->getOccupiedSpace();
    }
    else
        return 0;
}
