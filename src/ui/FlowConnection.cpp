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
    int arrowHeadSize = 10; // 箭头尖端大小
    double angle = atan2(d->m_pos2.y() - d->m_pos1.y(), d->m_pos2.x() - d->m_pos1.x()); // 计算箭头方向与x轴的夹角
    dx = arrowHeadSize * cos(angle - M_PI / 5); // 计算箭头左侧顶点的x坐标偏移
    dy = arrowHeadSize * sin(angle - M_PI / 5); // 计算箭头左侧顶点的y坐标偏移
    path.moveTo(d->m_pos2);
    path.lineTo(d->m_pos2.x() - dx, d->m_pos2.y() - dy); // 绘制箭头左侧顶点
    dx = arrowHeadSize * cos(angle + M_PI / 5); // 计算箭头右侧顶点的x坐标偏移
    dy = arrowHeadSize * sin(angle + M_PI / 5); // 计算箭头右侧顶点的y坐标偏移
    path.moveTo(d->m_pos2);
    path.lineTo(d->m_pos2.x() - dx, d->m_pos2.y() - dy); // 绘制箭头右侧顶点

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
    if (!d->m_text.isEmpty())
    {
        auto dx = (d->m_pos2.x() + d->m_pos1.x()) / 2;
        auto dy = (d->m_pos2.y() + d->m_pos1.y()) / 2;
        painter->setFont(d->m_font);
        painter->setPen(QPen(d->m_color));
        painter->drawText(dx, dy, d->m_text);
    }
}

QVariant FlowConnection::itemChange(GraphicsItemChange change, const QVariant& value)
{
    return value;
}
