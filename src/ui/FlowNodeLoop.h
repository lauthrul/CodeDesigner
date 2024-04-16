#pragma once

#include "FlowNode.h"

class FlowNodeLoop : public FlowNode
{
    Q_OBJECT
public:
    explicit FlowNodeLoop(const NodeInfo& data, QGraphicsItem* parent = 0);
    ~FlowNodeLoop();

public:
    virtual int type() const override { return FlowItemType::UINodeLoop; }
    virtual void setData(const NodeInfo& data) override;
    virtual void setPath(QRectF& rc) override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
