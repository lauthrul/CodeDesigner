﻿#pragma once

#include "FlowNode.h"

class FlowNodeCustomCode : public FlowNode
{
    Q_OBJECT
public:
    explicit FlowNodeCustomCode(const NodeInfo& data, QGraphicsItem* parent = 0);
    ~FlowNodeCustomCode();

public:
    virtual int type() const override { return FlowItemType::UINodeCustomCode; }
    virtual void setData(const NodeInfo& data) override;
    virtual void setPath(QRectF& rc) override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    struct Private;
    QSharedPointer<Private> d;
};
