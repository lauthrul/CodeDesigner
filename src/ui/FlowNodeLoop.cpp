#include "FlowNodeLoop.h"

struct FlowNodeLoop::Private
{
};

FlowNodeLoop::FlowNodeLoop(QGraphicsItem* parent /*= 0*/) :
    FlowNode(parent), d(new FlowNodeLoop::Private)
{
}

FlowNodeLoop::~FlowNodeLoop()
{

}

void FlowNodeLoop::setPath(QRectF& rect)
{
    QPainterPath path;
    auto w = rect.width() * 1.2;
    auto h = rect.height() * 1.2;
    rect.setWidth(w);
    rect.setHeight(h);
    path.addEllipse(rect);
    QGraphicsPathItem::setPath(path);
}

void FlowNodeLoop::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    __super::paint(painter, option, widget);
}
