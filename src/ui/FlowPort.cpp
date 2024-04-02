#include "FlowPort.h"
#include <QtMath>
#include <QKeyEvent>
#include <QApplication>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

struct FlowPort::Private
{
    // properties
    Direction m_direction;
    int m_size;
    QColor m_color;
    // datas
    QList<FlowConnection*> m_connections;
};

FlowPort::FlowPort(const Direction& direction, QGraphicsItem* parent /*= 0*/) :
    QGraphicsPathItem(parent), d(new FlowPort::Private)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setSize(4);
    setColor(Qt::red);
    setDirection(direction);
}

FlowPort::~FlowPort()
{
    for (auto con : d->m_connections)
        delete con;
    d->m_connections.clear();
}

Direction FlowPort::direction() const
{
    return d->m_direction;
}

void FlowPort::setDirection(const Direction& style)
{
    if (d->m_direction != style)
    {
        d->m_direction = style;
        update();
    }
}

int FlowPort::size() const
{
    return d->m_size;
}

void FlowPort::setSize(int size)
{
    if (d->m_size != size)
    {
        d->m_size = size;

        QPainterPath p;
        p.addEllipse(-d->m_size, -d->m_size, 2 * d->m_size, 2 * d->m_size);
        setPath(p);

        update();
    }
}

QColor FlowPort::color() const
{
    return d->m_color;
}

void FlowPort::setColor(const QColor& color)
{
    if (d->m_color != color)
    {
        d->m_color = color;
        setBrush(color);
        setPen(color.darker());
        update();
    }
}

QList<FlowConnection*>& FlowPort::connections()
{
    return d->m_connections;
}

bool FlowPort::isConnectedTo(const FlowPort* port) const
{
    for (auto con : d->m_connections)
    {
        if (con->port1() == port || con->port2() == port)
            return true;
    }
    return false;
}

QVariant FlowPort::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == GraphicsItemChange::ItemScenePositionHasChanged)
    {
        for (auto con : d->m_connections)
        {
            con->setPos1(con->port1()->scenePos());
            con->setPos2(con->port2()->scenePos());
        }
    }
    return value;
}
