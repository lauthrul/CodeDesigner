#include "FlowNodeFunction.h"

struct FlowNodeFunction::Private
{
};

FlowNodeFunction::FlowNodeFunction(const NodeInfo& data, QGraphicsItem* parent /*= 0*/) :
    FlowNode(data, parent), d(new FlowNodeFunction::Private)
{

}

FlowNodeFunction::~FlowNodeFunction()
{

}

void FlowNodeFunction::setData(const NodeInfo& data)
{
    if (data.type != NT_Function) return;

    setIcon(QIcon(data.icon));
    setText(data.scope());
    setExtend(data.function.type != FT_API);
    setPos(data.pos);

    if (ports().isEmpty())
    {
        addPort(new FlowPort(Top, IO::IN, "", this));
        addPort(new FlowPort(Bottom, IO::OUT, "", this));
    }

    __super::setData(data);
}

void FlowNodeFunction::setPath(QRectF& rect)
{
    QPainterPath path;
    path.addRoundedRect(rect, 5, 5);
    QGraphicsPathItem::setPath(path);
}

void FlowNodeFunction::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    __super::paint(painter, option, widget);
}
