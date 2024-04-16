#include "Models.h"
#include "DataManager.h"

//////////////////////////////////////////////////////////////////////////

QDataStream& operator<<(QDataStream& out, const Function::Param& data)
{
    out << data.type << data.name << data.value;
    return out;
}

QDataStream& operator>>(QDataStream& in, Function::Param& data)
{
    in >> data.type >> data.name >> data.value;
    return in;
}

QDataStream& operator<<(QDataStream& out, const Function& data)
{
    out << data.type << data.raw << data.retType << data.name << data.params.size();
    for (const auto& param : data.params)
        out << param;
    return out;
}

QDataStream& operator>>(QDataStream& in, Function& data)
{
    int size = 0;
    in >> data.type >> data.raw >> data.retType >> data.name >> size;
    for (int i = 0; i < size; i++)
    {
        Function::Param param;
        in >> param;
        data.params.append(param);
    }
    return in;
}

QDataStream& operator<<(QDataStream& out, const FunctionList& data)
{
    out << data.size();
    for (const auto& info : data)
        out << info;
    return out;
}

QDataStream& operator>>(QDataStream& in, FunctionList& data)
{
    int size = 0;
    in >> size;
    for (int i = 0; i < size; i++)
    {
        Function info;
        in >> info;
        data.append(info);
    }
    return in;
}

//////////////////////////////////////////////////////////////////////////
QMap<NodeType, QString> sNodeTypeMapping =
{
    { NT_Function, QT_TRANSLATE_NOOP("Models", "Function") },
    { NT_Condtion, QT_TRANSLATE_NOOP("Models", "Condition") },
    { NT_Loop, QT_TRANSLATE_NOOP("Models", "Loop") },
    { NT_CustomCode, QT_TRANSLATE_NOOP("Models", "CustomCode") },
};

NodeInfo NodeInfo::emptyNode = NodeInfo();

NodeInfo* NodeInfo::findChild(const QString& uid)
{
    NodeInfo* item = nullptr;
    traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->uid == uid)
        {
            item = node;
            return false;
        }
        return true;
    });
    return item;
}

NodeInfo* NodeInfo::findChildByName(const QString& name)
{
    NodeInfo* item = nullptr;
    traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->name == name)
        {
            item = node;
            return false;
        }
        return true;
    });
    return item;
}

void NodeInfo::addChild(const NodeInfo& node)
{
    children.push_back(node);
}

bool NodeInfo::removeChild(const QString& uid)
{
    bool ret = false;
    traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->uid == uid && parent != nullptr)
        {
            auto it = std::find(parent->children.begin(), parent->children.end(), *node);
            parent->children.erase(it);
            ret = true;
            return false;
        }
        return true;
    });
    return ret;
}

bool NodeInfo::removeChildByName(const QString& name)
{
    bool ret = false;
    traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->function.name == name && parent != nullptr)
        {
            auto it = std::find(parent->children.begin(), parent->children.end(), *node);
            parent->children.erase(it);
            ret = true;
            return false;
        }
        return true;
    });
    return ret;
}

bool NodeInfo::removeConnection(const QString& connectionstr)
{
    bool ret = false;
    traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
    {
        if (node->connections.contains(connectionstr))
        {
            node->connections.removeAll(connectionstr);
            ret = true;
            return false;
        }
        return true;
    });
    return ret;
}

//////////////////////////////////////////////////////////////////////////

QMap<NodeInfo::LoopType, QString> sLoopTypeMapping =
{
    {NodeInfo::WHILE, "while"},
    {NodeInfo::DO_WHILE, "do while"},
    {NodeInfo::FOR, "for"},
    {NodeInfo::FOR_EACH, "for each"},
};

bool operator==(const NodeInfo& lh, const NodeInfo& rh)
{
    return lh.uid == rh.uid;
}

QDataStream& operator<<(QDataStream& out, const NodeInfo& data)
{
    out << data.type << data.name << data.icon << data.uid << data.pos.x() << data.pos.y() << data.connections << data.function;
    out << (int)data.children.size();
    for (const auto& child : data.children)
        out << child;
    out << data.condition << data.loopType /*<< data.loopInitial << data.loopCondition << data.loopIterator*/;
    return out;
}

QDataStream& operator>>(QDataStream& in, NodeInfo& data)
{
    qreal x, y;
    in >> data.type >> data.name >> data.icon >> data.uid >> x >> y >> data.connections >> data.function;
    data.pos = { x, y };
    int size = 0;
    in >> size;
    for (int i = 0; i < size; i++)
    {
        NodeInfo child;
        in >> child;
        data.children.push_back(child);
    }
    in >> data.condition >> data.loopType /*>> data.loopInitial >> data.loopCondition >> data.loopIterator*/;
    return in;
}

QDataStream& operator<<(QDataStream& out, const NodeInfoList& data)
{
    out << (int)data.size();
    for (const auto& info : data)
        out << info;
    return out;
}

QDataStream& operator>>(QDataStream& in, NodeInfoList& data)
{
    int size = 0;
    in >> size;
    for (int i = 0; i < size; i++)
    {
        NodeInfo info;
        in >> info;
        data.push_back(info);
    }
    return in;
}

bool traverseNodeInfo(NodeInfo* node, NodeInfo* root, traverseNodeInfoFunc func, void* userData /*= nullptr*/)
{
    if (!func(node, root, userData)) return false;
    for (auto& child : node->children)
    {
        if (!traverseNodeInfo(&child, node, func, userData))
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

QJsonObject nodeInfoToJson(const NodeInfo& node)
{
    QJsonObject obj;
    obj["uid"] = node.uid;
    obj["type"] = node.type;
    obj["name"] = node.name;

    QJsonObject objPos;
    objPos["x"] = node.pos.x();
    objPos["y"] = node.pos.y();
    obj["pos"] = objPos;

    QJsonObject objFunction;
    objFunction["type"] = node.function.type;
    objFunction["raw"] = node.function.raw;
    objFunction["retType"] = node.function.retType;
    objFunction["name"] = node.function.name;
    QJsonArray arrParams;
    for (const auto& param : node.function.params)
    {
        QJsonObject objParam;
        objParam["type"] = param.type;
        objParam["name"] = param.name;
        objParam["value"] = param.value.toString();
        arrParams.append(objParam);
    }
    objFunction["params"] = arrParams;
    obj["function"] = objFunction;

    obj["icon"] = node.icon;

    QJsonArray  arrConnections;
    for (auto& conn : node.connections)
        arrConnections.append(QJsonValue(conn));
    obj["connections"] = arrConnections;

    obj["condition"] = node.condition;

    obj["loopType"] = node.loopType;
    //obj["loopInitial"] = node.loopInitial;
    //obj["loopCondition"] = node.loopCondition;
    //obj["loopIterator"] = node.loopIterator;

    QJsonArray objChildren;
    for (const auto& child : node.children)
        objChildren.append(nodeInfoToJson(child));
    obj["children"] = objChildren;
    return obj;
}

NodeInfo jsonToNodeInfo(const QJsonObject& obj)
{
    NodeInfo node;
    if (obj.isEmpty()) return node;

    if (obj.contains("uid")) node.uid = obj["uid"].toString();
    if (obj.contains("type")) node.type = (NodeType)obj["type"].toInt();
    if (obj.contains("name")) node.name = obj["name"].toString();
    if (obj.contains("pos"))
    {
        auto jv = obj["pos"];
        if (jv.isObject())
        {
            auto jo = jv.toObject();
            if (jo.contains("x")) node.pos.setX(jo["x"].toDouble());
            if (jo.contains("y")) node.pos.setY(jo["y"].toDouble());
        }
    }
    if (obj.contains("icon")) node.icon = obj["icon"].toString();
    if (obj.contains("function"))
    {
        auto jv = obj["function"];
        if (jv.isObject())
        {
            auto jo = jv.toObject();
            if (jo.contains("type")) node.function.type = (FunctionType)jo["type"].toInt();
            if (jo.contains("raw")) node.function.raw = jo["raw"].toString();
            if (jo.contains("retType")) node.function.retType = jo["retType"].toString();
            if (jo.contains("name")) node.function.name = jo["name"].toString();
            if (jo.contains("params"))
            {
                jv = jo["params"];
                if (jv.isArray())
                {
                    for (const auto& item : jv.toArray())
                    {
                        if (item.isObject())
                        {
                            jo = item.toObject();
                            Function::Param param;
                            if (jo.contains("type")) param.type = jo["type"].toString();
                            if (jo.contains("name")) param.name = jo["name"].toString();
                            if (jo.contains("value")) param.value = jo["value"].toString();
                            node.function.params.append(param);
                        }
                    }
                }
            }
        }
    }
    if (obj.contains("connections"))
    {
        auto jv = obj["connections"];
        if (jv.isArray())
        {
            for (const auto& item : jv.toArray())
                node.connections.append(item.toString());
        }
    }

    if (obj.contains("condition")) node.condition = obj["condition"].toString();

    if (obj.contains("loopType")) node.loopType = (NodeInfo::LoopType)obj["loopType"].toInt();
    //if (obj.contains("loopInitial")) node.loopInitial = obj["loopInitial"].toString();
    //if (obj.contains("loopCondition")) node.loopCondition = obj["loopCondition"].toString();
    //if (obj.contains("loopIterator")) node.loopIterator = obj["loopIterator"].toString();

    if (obj.contains("children"))
    {
        auto jv = obj["children"];
        if (jv.isArray())
        {
            for (const auto& item : jv.toArray())
            {
                if (item.isObject())
                    node.children.push_back(jsonToNodeInfo(item.toObject()));
            }
        }
    }
    return node;
}

//////////////////////////////////////////////////////////////////////////

QJsonArray varsToJson(const VariableList& vars)
{
    QJsonArray arr;
    for (const auto& var : vars)
    {
        QJsonObject obj;
        obj["name"] = var.name;
        obj["type"] = var.type;
        obj["arrSize"] = var.arrSize;
        obj["value"] = var.value;
        arr << obj;
    }
    return arr;
}

VariableList jsonToVars(const QJsonArray& arr)
{
    VariableList vars;
    for (const auto& jv : arr)
    {
        Variable var;
        if (jv.isObject())
        {
            auto jo = jv.toObject();
            if (jo.contains("name")) var.name = jo["name"].toString();
            if (jo.contains("type")) var.type = jo["type"].toString();
            if (jo.contains("arrSize")) var.arrSize = jo["arrSize"].toInt();
            if (jo.contains("value")) var.value = jo["value"].toString();
            vars << var;
        }
    }
    return vars;
}

//////////////////////////////////////////////////////////////////////////

QJsonArray binCodesToJson(const BinCodeList& bincodes)
{
    QJsonArray arr;
    for (const auto& bincode : bincodes)
    {
        QJsonObject obj;
        obj["key"] = bincode.first;
        obj["value"] = bincode.second;
        arr << obj;
    }
    return arr;
}

BinCodeList jsonToBinCodes(const QJsonArray& arr)
{
    BinCodeList bincodes;
    for (const auto& jv : arr)
    {
        if (jv.isObject())
        {
            BinCode bincode;
            auto jo = jv.toObject();
            if (jo.contains("key")) bincode.first = jo["key"].toString();
            if (jo.contains("value")) bincode.second = jo["value"].toInt();
            bincodes << bincode;
        }
    }
    return bincodes;
}