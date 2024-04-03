#include "DataManager.h"
#include "ui/TypeDefine.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

//////////////////////////////////////////////////////////////////////////

using FnRegexGetter = std::function<void(QRegularExpressionMatch& match, void* data)>;
void regex(const QString& text, const QString& reg, int option, void* data, FnRegexGetter fn)
{
    QRegularExpression re(reg, (QRegularExpression::PatternOption)option);
    QRegularExpressionMatchIterator i = re.globalMatch(text);
    QStringList funcs;
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        fn(match, data);
    }
}

QStringList regex(const QString& text, const QString& reg, int option /*= 0*/, int group /*= 1*/)
{
    QStringList funcs;
    regex(text, reg, option, &funcs, [ = ](QRegularExpressionMatch & match, void* data)
    {
        auto funcs = (QStringList*)data;
        funcs->append(match.captured(group));
    });
    return funcs;
}

//////////////////////////////////////////////////////////////////////////

struct DataManager::Private
{
    QString filePath;
    NodeInfo rootNode;
};

DataManager::DataManager(QObject* parent /*= nullptr*/) :
    d(new DataManager::Private)
{

}

DataManager::~DataManager()
{

}

DataManager* DataManager::instance()
{
    static DataManager inst;
    return &inst;
}

QString DataManager::genUUid()
{
    QUuid uid = QUuid::createUuid();
    return uid.toString();
}

FunctionList DataManager::loadFunctions(const QString& path)
{
    FunctionList functions;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return functions;

    QString content;
    QTextStream in(&file);
    content = in.readAll();
    regex(content, R"(TESTPLAN_API_C\s+(\w+)\s+(\w+)\s*\((.*)\);)",
          0, &functions, [](QRegularExpressionMatch & match, void* data)
    {
        auto funcs = (FunctionList*)data;
        Function func;
        func.type = Function::API;
        func.raw = match.captured(0);
        func.retType = match.captured(1);
        func.name = match.captured(2);
        auto params = match.captured(3);
        auto parts = params.split(",", Qt::SkipEmptyParts);
        if (!parts.isEmpty())
        {
            for (const auto& part : parts)
            {
                auto v = part.trimmed();
                v = v.mid(0, v.lastIndexOf("=")).trimmed();
                Function::Param param;
                auto pos = v.lastIndexOf(" ");
                param.type = v.mid(0, pos);
                param.name = v.mid(pos + 1);
                func.params.append(param);
            }
        }
        funcs->append(func);
    });
    regex(content, R"(#define\s+(\w+)\((.*?)\))",
          0, &functions, [](QRegularExpressionMatch & match, void* data)
    {
        auto funcs = (FunctionList*)data;
        Function func;
        func.type = Function::API;
        func.raw = match.captured(0);
        func.name = match.captured(1);
        auto params = match.captured(2);
        auto parts = params.split(",", Qt::SkipEmptyParts);
        if (!parts.isEmpty())
        {
            for (const auto& part : parts)
            {
                auto v = part.trimmed();
                v = v.mid(0, v.lastIndexOf("=")).trimmed();
                Function::Param param;
                auto pos = v.lastIndexOf(" ");
                param.type = v.mid(0, pos);
                param.name = v.mid(pos + 1);
                func.params.append(param);
            }
        }
        funcs->append(func);
    });
    return functions;
}

const FunctionList& DataManager::systemFunctions()
{
    static FunctionList functions =
    {
        {
            Function::System,
            "void TestFixtureSetup()",
            "void",
            "TestFixtureSetup",
        },
        {
            Function::System,
            "void TestFixtureTeardown()",
            "void",
            "TestFixtureTeardown",
        },
        {
            Function::System,
            "void TestItemInitialize()",
            "void",
            "TestItemInitialize",
        },
        {
            Function::System,
            "void TestItemFinalize()",
            "void",
            "TestItemFinalize",
        },
        {
            Function::System,
            "void TestFlow()",
            "void",
            "TestFlow",
        },
    };
    return functions;
}

bool DataManager::isSystemFunction(const QString& funcName)
{
    if (funcName == "main") return true;
    for (const auto& func : systemFunctions())
        if (func.name == funcName)
            return true;
    return false;
}

NodeInfo DataManager::createNodeInfo()
{
    NodeInfo root;
    root.uid = genUUid();
    root.function =
    {
        Function::System,
        "void main()",
        "void",
        "main",
    };
    auto i = 0;
    for (const auto& func : systemFunctions())
    {
        NodeInfo node;
        node.uid = genUUid();
        node.function = func;
        node.pos = {0, (qreal)DEFAULT_NODE_HEIGHT* i * 2 };
        root.children.push_back(node);
        i++;
    }
    for (i = 0; i < root.children.size() - 1; i++)
    {
        auto it1 = root.children.begin();
        auto it2 = root.children.begin();
        advance(it1, i);
        advance(it2, i + 1);
        if (it1 != root.children.end() && it2 != root.children.end())
        {
            root.connections.push_back(QString("%1_%2_%3_%4")
                                       .arg(it1->uid)
                                       .arg(Direction::Bottom)
                                       .arg(Direction::Top)
                                       .arg(it2->uid));
        }
    }
    return root;
}

NodeInfo jsonToNodeInfo(const QJsonObject& obj)
{
    NodeInfo node;
    if (obj.isEmpty()) return node;

    if (obj.contains("uid")) node.uid = obj["uid"].toString();
    if (obj.contains("pos"))
    {
        auto jv = obj["pos"];
        if (jv.isObject())
        {
            auto jo = jv.toObject();
            if (jo.contains("x")) node.pos.setX(jo["x"].toInt());
            if (jo.contains("y")) node.pos.setY(jo["y"].toInt());
        }
    }
    if (obj.contains("icon")) node.icon = QIcon(obj["icon"].toString());
    if (obj.contains("function"))
    {
        auto jv = obj["function"];
        if (jv.isObject())
        {
            auto jo = jv.toObject();
            if (jo.contains("type")) node.function.type = (Function::Type)jo["type"].toInt();
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

int DataManager::loadNodeInfo(NodeInfo& out, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;

    QTextStream in(&file);
    auto content = in.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(content.toLocal8Bit(), &err);
    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << QString("parse file[%1] error: %2").arg(path).arg(err.errorString());
        return -2;
    }

    if (!doc.isObject())
    {
        qDebug() << QString("invalid format: %1").arg(path);
        return -3;
    }

    out = jsonToNodeInfo(doc.object());
    return 0;
}

QJsonObject nodeInfoToJson(const NodeInfo& node)
{
    QJsonObject obj;
    obj["uid"] = node.uid;

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

    obj["icon"] = node.icon.themeName();

    QJsonArray  arrConnections;
    for (auto& conn : node.connections)
        arrConnections.append(QJsonValue(conn));
    obj["connections"] = arrConnections;

    QJsonArray objChildren;
    for (const auto& child : node.children)
        objChildren.append(nodeInfoToJson(child));
    obj["children"] = objChildren;
    return obj;
}

int DataManager::saveNodeInfo(const NodeInfo& node, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return -1;

    auto js = nodeInfoToJson(node);
    QJsonDocument doc(js);
    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    file.close();
    return 0;
}

bool DataManager::traverseNodeInfo(NodeInfo* node, NodeInfo* root, traverseNodeInfoFunc func, void* userData /*= nullptr*/)
{
    if (!func(node, root, userData)) return false;
    for (auto& sub : node->children)
    {
        if (!traverseNodeInfo(&sub, node, func, userData))
            return false;
    }
    return true;
}

void DataManager::setCurrentFilePath(const QString& path)
{
    d->filePath = path;
}

void DataManager::setCurrentRootNode(const NodeInfo& root)
{
    d->rootNode = root;
}

QString DataManager::currentFilePath() const
{
    return d->filePath;
}

NodeInfo& DataManager::currentRootNode() const
{
    return d->rootNode;
}

bool DataManager::saveCurrentNodeInfo()
{
    return saveNodeInfo(d->rootNode, d->filePath);
}
