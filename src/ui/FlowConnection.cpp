#include "FlowConnection.h"
#include "FlowPort.h"
#include <QtMath>
#include <QKeyEvent>
#include <QApplication>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

struct FlowConnection::Private
{
    QPointF m_pos1;
    QPointF m_pos2;
    FlowPort* m_port1 = nullptr;
    FlowPort* m_port2 = nullptr;
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

void FlowConnection::updatePath()
{
    QPainterPath path;
    //if (d->m_port1 && d->m_port2)
    //{

    //}
    //else
    {
        path.moveTo(d->m_pos1);

        qreal dx = d->m_pos2.x() - d->m_pos1.x();
        qreal dy = d->m_pos2.y() - d->m_pos1.y();

        QPointF ctr1(d->m_pos1.x() + dx * 0.25, d->m_pos1.y() + dy * 0.1);
        QPointF ctr2(d->m_pos1.x() + dx * 0.75, d->m_pos1.y() + dy * 0.9);

        path.cubicTo(ctr1, ctr2, d->m_pos2);

        //QPolygonF polygonf;
        //polygonf << d->m_pos2
        //         << QPointF(d->m_pos2.x() - 4, d->m_pos2.y() - 5)
        //         << QPointF(d->m_pos2.x() + 4, d->m_pos2.y() - 5)
        //         << d->m_pos2;
        //path.addPolygon(polygonf);
    }
    setPath(path);
}

void FlowConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    if (isSelected())
    {
        painter->setPen(QPen(Qt::red, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
    }
    else
    {
        painter->setPen(QPen(QColor(0, 122, 204), 2, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
    }
    updatePath();
    painter->drawPath(path());
}

QVariant FlowConnection::itemChange(GraphicsItemChange change, const QVariant& value)
{
    return value;
}
