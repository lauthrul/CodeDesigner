#include "DataManager.h"
#include "core/Models.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QUuid>
#include <QJsonDocument>
#include "com.h"
#include "../res/version.h"

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
    File file;
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
    regex(content, R"(\b(?:TESTPLAN_API_C|inline)\s+([\w<>:]+)\s+([\w*]+)\s*\(([^)]*)\)\s*)",
          0, &functions, [](QRegularExpressionMatch & match, void* data)
    {
        auto funcs = (FunctionList*)data;
        Function func;
        func.type = FT_API;
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
        func.type = FT_API;
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
            FT_System,
            "void TestFixtureSetup()",
            "void",
            "TestFixtureSetup",
        },
        {
            FT_System,
            "void TestFixtureTeardown()",
            "void",
            "TestFixtureTeardown",
        },
        {
            FT_System,
            "void TestItemInitialize()",
            "void",
            "TestItemInitialize",
        },
        {
            FT_System,
            "void TestItemFinalize()",
            "void",
            "TestItemFinalize",
        },
        {
            FT_System,
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

const NodeInfoList& DataManager::systemNodes()
{
    static NodeInfoList list =
    {
        {NodeType::NT_Condtion, tr("Condition"), ":/images/icon_condition.png"},
        {NodeType::NT_Loop, tr("Loop"), ":/images/icon_loop.png"},
        {NodeType::NT_CustomCode, tr("CustomCode"), ":/images/icon_custom_code.png"},
    };
    return list;
}

File DataManager::create()
{
    NodeInfo root;
    root.type = NT_Function;
    root.uid = genUUid();
    root.function =
    {
        FT_System,
        "void main()",
        "void",
        "main",
    };
    root.name = root.function.name;
    auto i = 0;
    for (const auto& func : systemFunctions())
    {
        NodeInfo node;
        node.icon = ":/images/icon_fx.png";
        node.type = NT_Function;
        node.uid = genUUid();
        node.name = func.name;
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
            root.connections.push_back(QString("%1_%2_%3_%4_%5")
                                       .arg((int)IO::OUT)
                                       .arg(it1->uid)
                                       .arg(Direction::Bottom)
                                       .arg(Direction::Top)
                                       .arg(it2->uid));
        }
    }
    File file;
    file.node = root;
    return file;
}

int DataManager::load(File& out, const QString& path)
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

    auto root = doc.object();

    if (root.contains("nodes") && root["nodes"].isObject())
        out.node = jsonToNodeInfo(root["nodes"].toObject());

    if (root.contains("vars") && root["vars"].isArray())
        out.vars = jsonToVars(root["vars"].toArray());

    if (root.contains("binCodes") && root["binCodes"].isObject())
    {
        auto jo = root["binCodes"].toObject();
        if (jo.contains("hBins") && jo["hBins"].isArray())
            out.hBins = jsonToBinCodes(jo["hBins"].toArray());
        if (jo.contains("sBins") && jo["sBins"].isArray())
            out.sBins = jsonToBinCodes(jo["sBins"].toArray());
    }

    if (root.contains("dec") && root["dec"].isObject())
    {
        auto jo = root["dec"].toObject();
        if (jo.contains("pinList") && jo["pinList"].isArray())
            out.pinList = jsonToPinList(jo["pinList"].toArray());
        if (jo.contains("timeSetList") && jo["timeSetList"].isArray())
            out.timeSetList = jsonToTimeSetList(jo["timeSetList"].toArray());
        if (jo.contains("pinGroups") && jo["pinGroups"].isArray())
            out.pinGroups = jsonToPinGroupList(jo["pinGroups"].toArray());
        if (jo.contains("powerPinGroups") && jo["powerPinGroups"].isArray())
            out.powerPinGroups = jsonToPinGroupList(jo["powerPinGroups"].toArray());
        if (jo.contains("urPinGroups") && jo["urPinGroups"].isArray())
            out.urPinGroups = jsonToPinGroupList(jo["urPinGroups"].toArray());
    }

    return 0;
}

int DataManager::save(const File& data, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return -1;

    QJsonObject root;
    root["nodes"] = nodeInfoToJson(data.node);
    root["vars"] = varsToJson(data.vars);
    {
        QJsonObject jo;
        jo["hBins"] = binCodesToJson(data.hBins);
        jo["sBins"] = binCodesToJson(data.sBins);
        root["binCodes"] = jo;
    }
    {
        QJsonObject jo;
        jo["pinList"] = pinListToJson(data.pinList);
        jo["timeSetList"] = timeSetListToJson(data.timeSetList);
        jo["pinGroups"] = pinGroupListToJson(data.pinGroups);
        jo["powerPinGroups"] = pinGroupListToJson(data.powerPinGroups);
        jo["urPinGroups"] = pinGroupListToJson(data.urPinGroups);
        root["dec"] = jo;
    }
    QJsonDocument doc(root);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toJson(QJsonDocument::Indented);
    file.close();
    return 0;
}

struct TreeNode
{
    QString id;
    bool visited;
    QMap<QString, QString> conns;
};

QString travseTree(const NodeInfo& node, QMap<QString, TreeNode>& tree,
                   const QString& intersection, const QString& end, const QString& id, int level)
{
    QString scope;
    if (id.isEmpty() || tree[id].visited) return scope;

    auto span = [&]() {return QString(level * 4, ' '); };
    auto item = ((NodeInfo*)&node)->findChild(id);
    if (item->type == NT_Function)
        scope = span() + item->scope() + ";\n";
    else
        scope = span() + item->scope() + "\n";
    if (item->type != NT_Condtion && item->type != NT_Loop)
        tree[id].visited = true;
    for (auto it = tree[id].conns.begin(); it != tree[id].conns.end(); it++)
    {
        auto dir = it.key(); // dir1_dir2.  0-left, 1-top, 2-right, 3-bottom
        auto toId = it.value();
        if (dir.startsWith("0")) // condition or loop start
        {
            scope += span() + "{\n";
            level++;
        }
        if (item->type == NT_Condtion)
        {
            if (dir.startsWith("2")) // condition else
            {
                tree[id].visited = true;
                level--;
                scope += span() + "}\n" + span() + "else\n" + span() + "{\n";
            }
        }
        if (item->type == NT_Loop)
        {
            if (dir.startsWith("2")) // exit loop
            {
                if (!tree[id].visited)
                {
                    tree[id].visited = true;
                    level--;
                    scope += span() + "}\n";
                }
                if (!intersection.isEmpty())
                    toId = "";
            }
        }
        if (dir.endsWith("3")) // end of loop
        {
            auto nextforloop = false;
            for (auto itj = tree[toId].conns.begin(); itj != tree[toId].conns.end(); itj++)
            {
                if (itj.key().startsWith("2")) // have next step after exit loop
                {
                    nextforloop = true;
                    break;
                }
            }
            if (!nextforloop)
            {
                tree[id].visited = true;
                level--;
                scope += span() + "}\n";
            }
            toId = "";
        }
        if (!intersection.isEmpty() && toId == intersection) // condition and loop intersection
        {
            //level--;
            scope += span() + "}\n";
        }
        scope += travseTree(node, tree, intersection, end, toId, level);
    }
    return scope;
}

QString DataManager::generateCode(const File& data, const QString& path, int* err)
{
    // open template
    QFile fTemplate(":/templates/code_template.text");
    if (!fTemplate.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (err) *err = -1;
        return "";
    }

    QTextStream in(&fTemplate);
    QString content = in.readAll();
    fTemplate.close();

    // version
    content.replace("$(AppName)", PRODUCT_NAME);
    content.replace("$(AppVersion)", FILE_VERSION_STR);

    // project name
    QFileInfo fi(path);
    content.replace("$(ProjectName)", fi.baseName());

    // bin codes
    QStringList scope;
    auto fnBin = [&](const BinCodeList & bins)
    {
        QStringList scope;
        for (const auto& bin : bins)
            scope << QString("    %1 = %2,").arg(bin.first)
                  .arg(bin.second > 0 ? QString::number(bin.second) : "~" + QString::number(~bin.second));
        return scope.join("\n");
    };
    content.replace("$(SBin)", fnBin(data.sBins));
    content.replace("$(HBin)", fnBin(data.hBins));

    // global variables
    scope.clear();
    for (const auto& var : data.vars)
    {
        if (var.arrSize > 1)
            scope << QString("static %1 %2[%3] = %4;").arg(var.type).arg(var.name).arg(var.arrSize).arg(var.value);
        else
            scope << QString("static %1 %2 = %3;").arg(var.type).arg(var.name).arg(var.value);
    }
    content.replace("$(GlobalVariables)", scope.join("\n"));

    // custom functions
    NodeInfoList systemNodes, customNodes;
    for (auto node : data.node.children)
    {
        if (node.type != NT_Function) continue;
        if (node.function.type == FT_System)
            systemNodes.push_back(node);
        else if (node.function.type == FT_Custom)
            customNodes.push_back(node);
    }
    auto fnFunction = [](const NodeInfoList & nodeList, bool system = false)
    {
        QStringList scope;
        for (auto& node : nodeList)
        {
            QString func = QString("%1%2\n{\n").arg(system ? "TESTPLAN_API_C " : "").arg(node.function.raw);
            QMap<QString, TreeNode> tree;
            QMap<QString, QList<QString>> counts1, counts2;
            QMap<QString, int> counts;
            for (const auto& conn : node.connections)
            {
                auto arr = conn.split("_");
                if (arr.size() >= 5)
                {
                    auto io = arr[0].toInt();
                    auto id1 = arr[1];
                    auto dir1 = arr[2];
                    auto dir2 = arr[3];
                    auto id2 = arr[4];
                    if (io == (int)IO::IN)
                    {
                        id1 = arr[4];
                        dir1 = arr[3];
                        dir2 = arr[2];
                        id2 = arr[1];
                    }
                    if (!tree.contains(id1))
                        tree[id1] = TreeNode{ id1, false };
                    tree[id1].conns[dir1 + "_" + dir2] = id2;
                    counts1[id1].push_back(dir1);
                    counts2[id2].push_back(dir2);
                    counts[id1]++;
                    counts[id2]++;
                }
            }
            if (tree.isEmpty())
            {
                if (node.children.size() == 1)
                {
                    auto uid = node.children[0].uid;
                    tree[uid] = {};
                    counts1[uid] = {};
                    counts[uid]++;
                }
            }
            QString start, end, intersection;
            for (auto it = counts.begin(); it != counts.end(); it++)
            {
                const auto& id = it.key();
                const auto& cnt = it.value();
                if (cnt == 1) // start or end point
                {
                    if (counts1.contains(id))
                        start = id;
                    else if (counts2.contains(id))
                        end = id;
                }
                if (cnt == 3) // condition or loop intersection
                {
                    if (counts2.contains(id) && counts2[id].size() == 2 && counts2[id][0] == counts2[id][1])
                        intersection = id;
                }
            }
            func += travseTree(node, tree, intersection, end, start, 1);
            func += "}\n";
            scope << func;
        }
        return scope;
    };
    content.replace("$(CustomFunctions)", fnFunction(customNodes).join("\n"));
    content.replace("$(SystemFunctions)", fnFunction(systemNodes, true).join("\n"));

    return content;
}

QString DataManager::generateDec(const File& file, const QString& path, int* err)
{
    // open template
    QFile fTemplate(":/templates/dec_template.text");
    if (!fTemplate.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (err) *err = -1;
        return "";
    }

    QTextStream in(&fTemplate);
    QString content = in.readAll();
    fTemplate.close();

    // version
    content.replace("$(AppName)", PRODUCT_NAME);
    content.replace("$(AppVersion)", FILE_VERSION_STR);

    // project name
    QFileInfo fi(path);
    content.replace("$(ProjectName)", fi.baseName());

    // PinList
    QStringList scope;
    for (const auto& pin : file.pinList)
    {
        QStringList sites;
        for (const auto& site : pin.siteIndexs)
            sites << QString::number(site);
        scope << QString("    %1 = %2 = %3 = %4;")
              .arg(pin.name).arg(sites.join(" : ")).arg(pin.dutIndex).arg(sPinTypeMapping[(int)pin.type]);
    }
    content.replace("$(PinList)", scope.join("\n"));

    // PinGroupList
    scope.clear();
    for (const auto& group : file.pinGroups)
    {
        QStringList pins;
        for (const auto& pin : group.second)
            pins << pin.split(",");
        scope << QString("    %1 = %2;").arg(group.first).arg(pins.join(" + "));
    }
    content.replace("$(PinGroupList)", scope.join("\n"));

    // TimeSetList
    scope.clear();
    for (const auto& tm : file.timeSetList)
        scope << QString("    %1 = %2;").arg(tm.name).arg(tm.interval);
    content.replace("$(TimeSetList)", scope.join("\n"));

    return content;
}

void DataManager::setPath(const QString& path)
{
    d->filePath = path;
}

void DataManager::setFile(const File& file)
{
    d->file = file;
}

void DataManager::setNode(const NodeInfo& node)
{
    d->file.node = node;
}

QString DataManager::path() const
{
    return d->filePath;
}

File& DataManager::file() const
{
    return d->file;
}

NodeInfo& DataManager::node() const
{
    return d->file.node;
}

VariableList& DataManager::vars() const
{
    return d->file.vars;
}

void DataManager::setVars(const VariableList& vars)
{
    d->file.vars = vars;
}

BinCodeList DataManager::sBinCodes()
{
    return d->file.sBins;
}

BinCodeList DataManager::hBinCodes()
{
    return d->file.hBins;
}

void DataManager::setBinCodes(const BinCodeList& sBins, const BinCodeList& hBins)
{
    d->file.sBins = sBins;
    d->file.hBins = hBins;
}

bool DataManager::save()
{
    return save(d->file, d->filePath);
}

QString DataManager::generateCode(bool save, __out int* err)
{
    auto code = generateCode(d->file, d->filePath, err);
    if (save)
    {
        QFileInfo fi(d->filePath);
        QString outPath = QString("%1/%2.cpp").arg(fi.path()).arg(fi.baseName());
        QFile of(outPath);
        if (!of.open(QIODevice::WriteOnly | QIODevice::Text))
            return -2;

        QTextStream os(&of);
        os << code;
        of.close();
    }
    return code;
}

QString DataManager::generateDec(bool save, __out int* err)
{
    auto code = generateDec(d->file, d->filePath, err);
    if (save)
    {
        QFileInfo fi(d->filePath);
        QString outPath = QString("%1/%2.dec").arg(fi.path()).arg(fi.baseName());
        QFile of(outPath);
        if (!of.open(QIODevice::WriteOnly | QIODevice::Text))
            return -2;

        QTextStream os(&of);
        os << code;
        of.close();
    }
    return code;
}
