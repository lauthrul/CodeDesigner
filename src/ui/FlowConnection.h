#pragma once

#include <QObject>
#include <QGraphicsPathItem>
#include "TypeDefine.h"

class FlowPort;
class FlowConnection : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    explicit FlowConnection(QGraphicsItem* parent = 0);
    ~FlowConnection();

public:
    virtual int type() const override { return FlowItemType::FTConnection; }

    void setPos1(const QPointF& p);
    void setPos2(const QPointF& p);
    void setPort1(FlowPort* p);
    void setPort2(FlowPort* p);
    FlowPort* port1() const;
    FlowPort* port2() const;
    void updatePath();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
