#include "FlowNodeFactory.h"
#include "FlowNodeFunction.h"
#include "FlowNodeCondition.h"
#include "FlowNodeLoop.h"

FlowNode* FlowNodeFactory::createFlowNode(const NodeInfo& nodeInfo)
{
	switch (nodeInfo.type)
	{
	case NodeType::NT_Function:
		return new FlowNodeFunction();
    case NodeType::NT_Condtion:
        return new FlowNodeCondition();
    case NodeType::NT_Loop:
        return new FlowNodeLoop();
	}
	return nullptr;
}
