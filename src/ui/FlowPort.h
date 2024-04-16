#pragma once

#include <QObject>
#include <QGraphicsPathItem>
#include "core/Models.h"
#include "FlowConnection.h"

class FlowPort : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
    Q_ENUM(Direction)
    Q_ENUM(IO)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection)
    Q_PROPERTY(IO io READ io WRITE setIO)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(int size READ size WRITE setSize)
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    explicit FlowPort(const Direction& direction, IO io, const QString& text, QGraphicsItem* parent = 0);
    ~FlowPort();

public:
    virtual int type() const override { return FlowItemType::UIPort; }

    Direction direction() const;
    void setDirection(const Direction& direction);
    IO io() const;
    void setIO(IO io);
    QString text();
    void setText(const QString& text);
    int size() const;
    void setSize(int size);
    QColor color() const;
    void setColor(const QColor& color);
    QList<FlowConnection*>& connections();
    bool isConnectedTo(const FlowPort* port) const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
