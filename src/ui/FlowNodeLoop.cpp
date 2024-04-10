#include "FlowNodeLoop.h"

struct FlowNodeLoop::Private
{
};

FlowNodeLoop::FlowNodeLoop(const NodeInfo& data, QGraphicsItem* parent /*= 0*/) :
    FlowNode(data, parent), d(new FlowNodeLoop::Private)
{

}

FlowNodeLoop::~FlowNodeLoop()
{

}

void FlowNodeLoop::setData(const NodeInfo& data)
{
    if (data.type != NT_Loop) return;

    setBackgroundColor(QColor(0xF9BE4D));
    QString text = data.name;
    switch (data.loopType)
    {
        case NodeInfo::FOR:
        case NodeInfo::FOR_EACH:
        case NodeInfo::WHILE:
            text = QString("%1 (%2)").arg(sLoopTypeMapping[data.loopType]).arg(data.condition);
            break;
        case NodeInfo::DO_WHILE:
            text = QString("do {...} while (%1)").arg(data.condition);
            break;
    }
    setText(text);
    setPos(data.pos);
    setExtend(false);

    __super::setData(data);
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
