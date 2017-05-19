#include "LayoutPolicy.h"
#include "scene/DisplayGroup.h"
// TODO put back the const
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

qreal LayoutPolicy::_computeMaxRatio(ContentWindowPtr window) const
{
    return std::max(window->width() / _getAvailableSpace().width(),
                    window->height() / _getAvailableSpace().height());
}

ContentWindowPtrs LayoutPolicy::_sortByMaxRatio(
    const ContentWindowSet& windows) const
{
    std::vector<ContentWindowPtr> windowVec;
    for (auto window : windows)
    {
        windowVec.push_back(window);
    }
    std::sort(windowVec.begin(), windowVec.end(),
              [this](ContentWindowPtr a, ContentWindowPtr b) {
                  return _computeMaxRatio(a) > _computeMaxRatio(b);
              });
    return windowVec;
}

QRectF LayoutPolicy::_addMargins(const ContentWindowPtr window)
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

QRectF LayoutPolicy::rectWithoutMargins(const QRectF& rect,
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
