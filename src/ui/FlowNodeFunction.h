#pragma once

#include "FlowNode.h"

class FlowNodeFunction : public FlowNode
{
    Q_OBJECT
public:
    explicit FlowNodeFunction(const NodeInfo& data, QGraphicsItem* parent = 0);
    ~FlowNodeFunction();

public:
    virtual int type() const override { return FlowItemType::UINodeFunction; }
    virtual void setData(const NodeInfo& data) override;
    virtual void setPath(QRectF& rc) override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
