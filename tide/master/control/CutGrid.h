#ifndef CUTGRID_H
#define CUTGRID_H
#include "Cut.h"
#include "CutPoint.h"
#include "CutRect.h"
#include "types.h"
#include <boost/enable_shared_from_this.hpp>
class CutGrid
{
public:
    CutGrid();
    std::vector<CutPointPtr> getPossibleInsertionPointsForWindow(
        ContentWindowPtr window) const;
    bool insertWindowAtPoint(CutPointPtr point, ContentWindowPtr window);
    bool isPossibleInsertWindowAtPoint(CutPointPtr point,
                                       ContentWindowPtr window) const;
    void balance();
    QRectF currentSize() const;

private:
    bool _intersectWithPreviousWindows(const QRectF& rect) const;
    void _addCut(CutPtr);
    bool _removeInsertionPoint(CutPointPtr point);
    void _augmentRight(int id, std::vector<std::vector<int>>& matrixOfIds,
                       std::vector<boost::shared_ptr<int>> indices) const;
    void _augmentLeft(int id, std::vector<std::vector<int>>& matrixOfIds,
                      std::vector<boost::shared_ptr<int>> indices) const;
    void _augmentTop(int id, std::vector<std::vector<int>>& matrixOfIds,
                     std::vector<boost::shared_ptr<int>> indices) const;
    void _augmentBottom(int id, std::vector<std::vector<int>>& matrixOfIds,
                        std::vector<boost::shared_ptr<int>> indices) const;
    std::vector<boost::shared_ptr<int>> _getIndicesInMatrix(
        const CutRectPtr cutRect) const;
    void _changeCutsWithNewIndicesOfMatrix(
        const std::vector<boost::shared_ptr<int>>& indices, int windowIndex);
    void _updateWindows();
    std::vector<CutPtr> heightCuts;
    std::vector<CutPtr> widthCuts;
    std::vector<CutPointPtr> insertionPoints;
    std::vector<CutRectPtr> windowsAdded;
};

#endif // CUTGRID_H
