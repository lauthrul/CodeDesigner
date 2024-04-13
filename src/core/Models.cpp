#include "Models.h"
#include "DataManager.h"

//////////////////////////////////////////////////////////////////////////
QMap<FunctionType, QString> sFunctionTypeMapping =
{
    { FT_System, TR("System") },
    { FT_API, TR("API") },
    { FT_Custom, TR("Custom") },
};

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
    { NT_Function, TR("Function") },
    { NT_Condtion, TR("Condition") },
    { NT_Loop, TR("Loop") },
    { NT_CustomCode, TR("CustomCode") },
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