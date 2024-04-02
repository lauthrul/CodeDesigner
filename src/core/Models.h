#pragma once

#include <QVariant>
#include <QDataStream>
#include <QIcon>

//////////////////////////////////////////////////////////////////////////
// 函数定义
struct Function
{
    enum Type { System, API, Custom, };
    Type type;
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
struct NodeInfo;
using NodeInfoList = std::list<NodeInfo>;
struct NodeInfo
{
    QString uid;                                // 唯一标识
    Function function;                          // 函数信息
    QIcon icon = QIcon(":/images/icon_fx.png"); // 图标
    QPointF pos = { 0, 0 };                     // 位置
    QStringList connections;                    // 连接信息 <uid_direction_direction_uid>
    NodeInfoList children;                      // 子节点

    static NodeInfo emptyNode;

    NodeInfo* find(const QString& uid);
    void add(const NodeInfo& node);
    bool removeByUid(const QString& uid);
    bool removeByName(const QString& name);
};

bool operator==(const NodeInfo& lh, const NodeInfo& rh);

