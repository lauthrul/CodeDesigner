#pragma once

#include "FlowNode.h"

class FlowNodeFunction : public FlowNode
{
public:
    explicit FlowNodeFunction(QGraphicsItem* parent = 0);
    ~FlowNodeFunction();

public:
    virtual int type() const override { return FlowItemType::NodeFunctionType; }
    void setPath(QRectF& rc);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
