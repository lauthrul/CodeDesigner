#include "DataManager.h"
#include "core/Models.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QUuid>
#include <QJsonDocument>

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
                                       .arg(OUT)
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
    QJsonObject jo;
    jo["hBins"] = binCodesToJson(data.hBins);
    jo["sBins"] = binCodesToJson(data.sBins);
    root["binCodes"] = jo;
    QJsonDocument doc(root);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toJson(QJsonDocument::Indented);
    file.close();
    return 0;
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
