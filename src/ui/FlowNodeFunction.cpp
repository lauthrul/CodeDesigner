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
    {
        auto value = val.value.toString();
        if (data.function.name == "Test")
        {
            auto arr = value.split(";");
            if (arr.size() >= 3)
            {
                value = arr[0].mid(arr[0].indexOf("-") + 1);
                if (!arr[1].isEmpty()) value += "(" + arr[1].mid(arr[1].indexOf("-") + 1);
                if (!arr[2].isEmpty()) value += "," + arr[2].mid(arr[2].indexOf("-") + 1) + ")";
            }
        }
        values << value;
    }
    if (!values.isEmpty())
        text += QString("(%1)").arg(values.join(", "));
    setText(text);

    setExtend(data.function.type != FT_API);
    setPos(data.pos);

    if (ports().isEmpty())
    {
        addPort(new FlowPort(Top, IN, "", this));
        addPort(new FlowPort(Bottom, OUT, "", this));
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
