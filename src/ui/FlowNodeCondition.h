#pragma once

#include "FlowNode.h"

class FlowNodeCondition : public FlowNode
{
    Q_OBJECT
public:
    explicit FlowNodeCondition(const NodeInfo& data, QGraphicsItem* parent = 0);
    ~FlowNodeCondition();

public:
    virtual int type() const override { return FlowItemType::UINodeCondtion; }
    virtual void setData(const NodeInfo& data) override;
    virtual void setPath(QRectF& rc) override;

protected:
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
