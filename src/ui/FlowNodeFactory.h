#pragma once

#include "FlowNode.h"
#include "core/Models.h"

class FlowNodeFactory
{
public:
    explicit FlowNodeFactory() = default;
    ~FlowNodeFactory() = default;

    static FlowNode* createFlowNode(const NodeInfo& nodeInfo);
};
