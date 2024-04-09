#pragma once

#include "FlowNode.h"

class FlowNodeLoop : public FlowNode
{
public:
    explicit FlowNodeLoop(QGraphicsItem* parent = 0);
    ~FlowNodeLoop();

public:
    virtual int type() const override { return FlowItemType::NodeLoopType; }
    void setPath(QRectF& rc);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
