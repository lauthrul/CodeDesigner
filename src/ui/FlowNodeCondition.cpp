#include "FlowNodeCondition.h"

struct FlowNodeCondition::Private
{
};

FlowNodeCondition::FlowNodeCondition(QGraphicsItem* parent /*= 0*/) :
    FlowNode(parent), d(new FlowNodeCondition::Private)
{
}

FlowNodeCondition::~FlowNodeCondition()
{

}

void FlowNodeCondition::setPath(QRectF& rect)
{
    QPainterPath path;
    auto w = rect.width() * 1.2;
    auto h = rect.height() * 1.2;
    rect.setWidth(w);
    rect.setHeight(h);
    QPolygonF polygonf;
    polygonf << QPointF(0, h / 2)
        << QPointF(w / 2, h)
        << QPointF(w, h / 2)
        << QPointF(w / 2, 0)
        << QPointF(0, h / 2);
    path.addPolygon(polygonf);
    QGraphicsPathItem::setPath(path);
}

void FlowNodeCondition::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    __super::paint(painter, option, widget);
}
