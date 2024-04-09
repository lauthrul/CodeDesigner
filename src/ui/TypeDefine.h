#pragma once
#include <QGraphicsItem>

enum FlowItemType
{
    NodeFunctionType = QGraphicsItem::UserType + 1000,
    NodeCondtionType,
    NodeLoopType,

    PortType,
    ConnectionType,
};

inline bool isFlowNodeType(int type)
{
    return type >= NodeFunctionType && type <= NodeLoopType;
}

enum Direction
{
    Left, Top, Right, Bottom, VCenter, HCenter,
};

const auto DEFAULT_NODE_WIDTH = 300;
const auto DEFAULT_NODE_HEIGHT = 48;