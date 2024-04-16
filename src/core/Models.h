#pragma once

#include <QVariant>
#include <QDataStream>
#include <QIcon>

//////////////////////////////////////////////////////////////////////////
// 函数类型
enum FunctionType
{
    FT_System, FT_API, FT_Custom,
};

// 函数定义
struct Function
{
    FunctionType type;  // 函数类型
    QString raw;        // 函数原型
    QString retType;    // 返回类型
    QString name;       // 函数名
    struct Param
    {
        QString type;   // 参数类型
        QString name;   // 参数名
        QVariant value; // 参数值
    };
    using ParamList = QList<Param>;
    ParamList params;
};
Q_DECLARE_METATYPE(Function);

using FunctionList = QList<Function>;

QDataStream& operator<<(QDataStream& out, const Function::Param& data);
QDataStream& operator>>(QDataStream& in, Function::Param& data);
QDataStream& operator<<(QDataStream& out, const Function& data);
QDataStream& operator>>(QDataStream& in, Function& data);
QDataStream& operator<<(QDataStream& out, const FunctionList& data);
QDataStream& operator>>(QDataStream& in, FunctionList& data);

//////////////////////////////////////////////////////////////////////////
// 节点定义

// 节点类型
enum NodeType
{
    NT_Function, NT_Condtion, NT_Loop, NT_CustomCode,
};
extern QMap<NodeType, QString> sNodeTypeMapping;

// 节点定义
struct NodeInfo;
using NodeInfoList = std::list<NodeInfo>;
struct NodeInfo
{
public: // 通用节点类型
    NodeType type;                              // 类型
    QString name;                               // 名称
    QString icon;                               // 图标
    QString uid;                                // 唯一标识
    QPointF pos = { 0, 0 };                     // 位置
    QStringList connections;                    // 连接信息 <uid_direction_direction_uid>

    static NodeInfo emptyNode;

public: // 连线使用
    bool removeConnection(const QString& connectionstr);

public: // 函数节点类型使用
    Function function;                          // 函数信息
    NodeInfoList children;                      // 子节点

    NodeInfo* findChild(const QString& uid);
    NodeInfo* findChildByName(const QString& name);
    void addChild(const NodeInfo& node);
    bool removeChild(const QString& uid);
    bool removeChildByName(const QString& name);

public: // 条件/循环使用
    QString condition;

public: // 循环使用
    enum LoopType { FOR, FOR_EACH, WHILE, DO_WHILE, };
    LoopType loopType;
//    QString loopInitial;
//    QString loopCondition;
//    QString loopIterator;
};
Q_DECLARE_METATYPE(NodeInfo);

extern QMap<NodeInfo::LoopType, QString> sLoopTypeMapping;

bool operator==(const NodeInfo& lh, const NodeInfo& rh);

QDataStream& operator<<(QDataStream& out, const NodeInfo& data);
QDataStream& operator>>(QDataStream& in, NodeInfo& data);
QDataStream& operator<<(QDataStream& out, const NodeInfoList& data);
QDataStream& operator>>(QDataStream& in, NodeInfoList& data);

using  traverseNodeInfoFunc = std::function<bool(NodeInfo* node, NodeInfo* root, void* userData)>;
bool traverseNodeInfo(NodeInfo* node, NodeInfo* root, traverseNodeInfoFunc func, void* userData = nullptr);

//////////////////////////////////////////////////////////////////////////
// 变量定义
struct Variable
{
    QString name;
    QString type;
    int arrSize;
    QString value;
};
using VariableList = QList<Variable>;

//////////////////////////////////////////////////////////////////////////
// 文件定义
struct File
{
    NodeInfo node;
    VariableList vars;
};

//////////////////////////////////////////////////////////////////////////
// UI数据定义

#include <QGraphicsItem>

enum FlowItemType
{
    __UINodeStart = QGraphicsItem::UserType + 1000,
    UINodeFunction,
    UINodeCondtion,
    UINodeLoop,
    UINodeCustomCode,
    __UINodeEnd,

    UIPort,
    UIConnection,
};

inline bool isFlowNodeType(int type)
{
    return type > __UINodeStart && type < __UINodeEnd;
}

enum IO
{
    IN, OUT
};

enum Direction
{
    Left, Top, Right, Bottom, VCenter, HCenter,
};

const auto DEFAULT_NODE_WIDTH = 300;
const auto DEFAULT_NODE_HEIGHT = 48;