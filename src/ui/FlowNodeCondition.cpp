﻿#include "FlowNodeCondition.h"

struct FlowNodeCondition::Private
{
};

FlowNodeCondition::FlowNodeCondition(const NodeInfo& data, QGraphicsItem* parent /*= 0*/) :
    FlowNode(data, parent), d(new FlowNodeCondition::Private)
{

}

FlowNodeCondition::~FlowNodeCondition()
{

}

void FlowNodeCondition::setData(const NodeInfo& data)
{
    if (data.type != NT_Condtion) return;

    setBackgroundColor(QColor(0xF9BE4D));
    auto text = data.name;
    if (!data.condition.isEmpty())
        text = QString("if (%1)").arg(data.condition);
    setText(text);
    setPos(data.pos);
    setExtend(false);

    __super::setData(data);
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
