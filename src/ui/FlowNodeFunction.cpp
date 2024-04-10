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

    auto text = data.function.name;
    QStringList values;
    for (const auto& val : data.function.params)
        values << val.value.toString();
    if (!values.isEmpty())
        text += QString("(%1)").arg(values.join(", "));
    setText(text);

    setPos(data.pos);
    setExtend(data.function.type != FT_API);

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
