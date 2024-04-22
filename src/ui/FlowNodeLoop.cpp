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
    setText(data.scope());
    setExtend(false);
    setPos(data.pos);

    if (ports().isEmpty())
    {
        addPort(new FlowPort(Left, IO::OUT, tr("Enter Loop"), this));
        addPort(new FlowPort(Top, IO::IN, "", this));
        addPort(new FlowPort(Right, IO::OUT, tr("Exit Loop"), this));
        addPort(new FlowPort(Bottom, IO::IN, tr("Back Loop"), this));
    }

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
