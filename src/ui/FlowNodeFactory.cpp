#include "FlowNodeFactory.h"
#include "FlowNodeFunction.h"
#include "FlowNodeCondition.h"
#include "FlowNodeLoop.h"
#include "FlowNodeCustomCode.h"

FlowNode* FlowNodeFactory::createFlowNode(const NodeInfo& nodeInfo)
{
    switch (nodeInfo.type)
    {
        case NodeType::NT_Function:
            return new FlowNodeFunction(nodeInfo);
        case NodeType::NT_Condtion:
            return new FlowNodeCondition(nodeInfo);
        case NodeType::NT_Loop:
            return new FlowNodeLoop(nodeInfo);
        case NodeType::NT_CustomCode:
            return new FlowNodeCustomCode(nodeInfo);
    }
    return nullptr;
}
