#pragma once
#include <QGraphicsItem>

enum FlowItemType
{
    FTNode = QGraphicsItem::UserType + 1000,
    FTPort,
    FTConnection,
};

enum Direction
{
    Left, Top, Right, Bottom,
};

const auto DEFAULT_NODE_WIDTH = 300;
const auto DEFAULT_NODE_HEIGHT = 48;