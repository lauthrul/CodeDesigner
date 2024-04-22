#include "FlowConnection.h"
#include "FlowPort.h"
#include <QtMath>
#include <QKeyEvent>
#include <QApplication>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

struct FlowConnection::Private
{
    QColor m_color = Qt::black;
    QPointF m_pos1;
    QPointF m_pos2;
    FlowPort* m_port1 = nullptr;
    FlowPort* m_port2 = nullptr;
    IO m_type = IO::OUT;
    QFont m_font = QFont("Microsoft YaHei", 10);
    QString m_text;
};

FlowConnection::FlowConnection(QGraphicsItem* parent /*= 0*/) :
    QGraphicsPathItem(parent), d(new FlowConnection::Private)
{
    setZValue(-1);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

FlowConnection::~FlowConnection()
{
    if (d->m_port1)
        d->m_port1->connections().removeAll(this);
    if (d->m_port2)
        d->m_port2->connections().removeAll(this);
}

QColor FlowConnection::color() const
{
    return d->m_color;
}

void FlowConnection::setColor(const QColor& c)
{
    if (d->m_color != c)
    {
        d->m_color = c;
        update();
    }
}

void FlowConnection::setPos1(const QPointF& p)
{
    if (d->m_pos1 != p)
    {
        d->m_pos1 = p;
        update();
    }
}

void FlowConnection::setPos2(const QPointF& p)
{
    if (d->m_pos2 != p)
    {
        d->m_pos2 = p;
        update();
    }
}

void FlowConnection::setPort1(FlowPort* p)
{
    if (d->m_port1 != p)
    {
        p->connections() << this;
        d->m_port1 = p;
        d->m_pos1 = p->scenePos();
        update();
    }
}

void FlowConnection::setPort2(FlowPort* p)
{
    if (d->m_port2 != p)
    {
        p->connections() << this;
        d->m_port2 = p;
        d->m_pos2 = p->scenePos();
        update();
    }
}

FlowPort* FlowConnection::port1() const
{
    return d->m_port1;
}

FlowPort* FlowConnection::port2() const
{
    return d->m_port2;
}

IO FlowConnection::connectionType() const
{
    return d->m_type;
}

void FlowConnection::setConnectionType(IO type)
{
    if (d->m_type != type)
    {
        d->m_type = type;
        update();
    }
}

bool FlowConnection::isLinked(FlowItemType nodeType, Direction dir)
{
    bool flag = false;
    if (d->m_port1)
        flag |= (d->m_port1->parentItem()->type() == nodeType) && d->m_port1->direction() == dir;
    if (d->m_port2)
        flag |= (d->m_port2->parentItem()->type() == nodeType) && d->m_port2->direction() == dir;
    return flag;
}

QString FlowConnection::text() const
{
    return d->m_text;
}

void FlowConnection::setText(const QString& text)
{
    if (d->m_text != text)
    {
        d->m_text = text;
        update();
    }
}

void FlowConnection::updatePath()
{
    QPainterPath path;

    // 绘制线段
    path.moveTo(d->m_pos1);
    qreal dx = d->m_pos2.x() - d->m_pos1.x();
    qreal dy = d->m_pos2.y() - d->m_pos1.y();
    QPointF ctr1(d->m_pos1.x() + dx * 0.25, d->m_pos1.y() + dy * 0.1);
    QPointF ctr2(d->m_pos1.x() + dx * 0.75, d->m_pos1.y() + dy * 0.9);
    path.cubicTo(ctr1, ctr2, d->m_pos2);

    // 绘制箭头
    auto start = (d->m_type == IO::OUT) ? d->m_pos1 : d->m_pos2;
    auto end = (d->m_type == IO::OUT) ? d->m_pos2 : d->m_pos1;
    int arrowHeadSize = 10; // 箭头尖端大小
    double angle = atan2(end.y() - start.y(), end.x() - start.x()); // 计算箭头方向与x轴的夹角
    dx = arrowHeadSize * cos(angle - M_PI / 5); // 计算箭头左侧顶点的x坐标偏移
    dy = arrowHeadSize * sin(angle - M_PI / 5); // 计算箭头左侧顶点的y坐标偏移
    path.moveTo(end);
    path.lineTo(end.x() - dx, end.y() - dy); // 绘制箭头左侧顶点
    dx = arrowHeadSize * cos(angle + M_PI / 5); // 计算箭头右侧顶点的x坐标偏移
    dy = arrowHeadSize * sin(angle + M_PI / 5); // 计算箭头右侧顶点的y坐标偏移
    path.moveTo(end);
    path.lineTo(end.x() - dx, end.y() - dy); // 绘制箭头右侧顶点

    setPath(path);
}

void FlowConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    // 绘制线条
    if (isSelected())
    {
        painter->setPen(QPen(Qt::red, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
    }
    else
    {
        painter->setPen(QPen(d->m_color, 2, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
    }
    updatePath();
    painter->drawPath(path());

    // 绘制文本
    auto text = d->m_text;
    if (text.isEmpty())
    {
        if (port1() && !port1()->text().isEmpty())
            text = port1()->text();
        else if (port2() && !port2()->text().isEmpty())
            text = port2()->text();
    }
    if (!text.isEmpty())
    {
        auto dx = (d->m_pos2.x() + d->m_pos1.x()) / 2;
        auto dy = (d->m_pos2.y() + d->m_pos1.y()) / 2;
        painter->setFont(d->m_font);
        painter->drawText(dx, dy, text);
    }
}

QVariant FlowConnection::itemChange(GraphicsItemChange change, const QVariant& value)
{
    return value;
}
