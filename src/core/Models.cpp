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
    out << data.raw << data.retType << data.name << data.params.size();
    for (const auto& param : data.params)
        out << param;
    return out;
}

QDataStream& operator>>(QDataStream& in, Function& data)
{
    int size = 0;
    in >> data.raw >> data.retType >> data.name >> size;
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

bool operator==(const NodeInfo& lh, const NodeInfo& rh)
{
    return lh.uid == rh.uid;
}

NodeInfo NodeInfo::emptyNode = NodeInfo();

NodeInfo* NodeInfo::find(const QString& uid)
{
    NodeInfo* item = nullptr;
    DataManager::traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
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

void NodeInfo::add(const NodeInfo& node)
{
    children.push_back(node);
}

bool NodeInfo::removeByUid(const QString& uid)
{
    bool ret = false;
    DataManager::traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
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

bool NodeInfo::removeByName(const QString& name)
{
    bool ret = false;
    DataManager::traverseNodeInfo(this, nullptr, [&](NodeInfo * node, NodeInfo * parent, void* userData)
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
