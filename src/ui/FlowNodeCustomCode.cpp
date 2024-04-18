#include "FlowNodeCustomCode.h"

struct FlowNodeCustomCode::Private
{
};

FlowNodeCustomCode::FlowNodeCustomCode(const NodeInfo& data, QGraphicsItem* parent /*= 0*/) :
    FlowNode(data, parent), d(new FlowNodeCustomCode::Private)
{

}

FlowNodeCustomCode::~FlowNodeCustomCode()
{

}

void FlowNodeCustomCode::setData(const NodeInfo& data)
{
    if (data.type != NT_CustomCode) return;

    setBackgroundColor(QColor(0x86A65D));
    setIcon(QIcon(data.icon));
    setText(data.scope());
    setExtend(false);
    setPos(data.pos);

    if (ports().isEmpty())
    {
        addPort(new FlowPort(Top, IN, "", this));
        addPort(new FlowPort(Bottom, OUT, "", this));
    }

    __super::setData(data);
}

void FlowNodeCustomCode::setPath(QRectF& rect)
{
    QPainterPath path;
    path.addRoundedRect(rect, 5, 5);
    QGraphicsPathItem::setPath(path);
}

void FlowNodeCustomCode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    __super::paint(painter, option, widget);
}
