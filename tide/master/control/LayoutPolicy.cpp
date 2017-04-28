#include "LayoutPolicy.h"
#include "scene/DisplayGroup.h"

using namespace controlSpecifications;
LayoutPolicy::LayoutPolicy(const DisplayGroup& group)
    : _group(group)
{
}

QRectF LayoutPolicy::_getAvailableSpace() const
{
    qreal left_width_margin =
        _group.width() * INSIDE_MARGIN_RELATIVE +
        _group.height() * SIDEBAR_WIDTH_REL_TO_DISPLAYGROUP_HEIGHT;
    qreal right_width_margin = _group.width() * INSIDE_MARGIN_RELATIVE;
    qreal top_margin = _group.height() * INSIDE_MARGIN_RELATIVE;
    qreal bottom_margin = top_margin;
    return QRectF(left_width_margin, top_margin,
                  _group.width() - left_width_margin - right_width_margin,
                  _group.height() - top_margin - bottom_margin);
}
