#pragma once
#include <QGraphicsItem>

enum FlowItemType
{
    __NodeTypeStart = QGraphicsItem::UserType + 1000,
    NodeFunctionType,
    NodeCondtionType,
    NodeLoopType,
    NodeCustomCodeType,
    __NodeTypeEnd,

    PortType,
    ConnectionType,
};

inline bool isFlowNodeType(int type)
{
    return type > __NodeTypeStart && type < __NodeTypeEnd;
}

enum Direction
{
    Left, Top, Right, Bottom, VCenter, HCenter,
};

const auto DEFAULT_NODE_WIDTH = 300;
const auto DEFAULT_NODE_HEIGHT = 48;