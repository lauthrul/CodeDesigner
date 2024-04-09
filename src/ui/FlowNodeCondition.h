#pragma once

#include "FlowNode.h"

class FlowNodeCondition : public FlowNode
{
public:
    explicit FlowNodeCondition(QGraphicsItem* parent = 0);
    ~FlowNodeCondition();

public:
    virtual int type() const override { return FlowItemType::NodeCondtionType; }
    void setPath(QRectF& rc);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
