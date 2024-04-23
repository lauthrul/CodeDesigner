#pragma once

#include <QVariant>
#include <QDataStream>
#include <QIcon>
#include <QJsonObject>
#include <QJsonArray>

//////////////////////////////////////////////////////////////////////////

using EnumStrMap = QMap<int, QString>;
#define ENUM_STR_MAP_ITEM(ENUM, E) { (int)ENUM::E, #E }

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
extern EnumStrMap sNodeTypeMapping;

// 节点定义
struct NodeInfo;
using NodeInfoList = QList<NodeInfo>;
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
    QString scope() const;

public: // 条件/循环使用
    QString condition;

public: // 循环使用
    enum LoopType { FOR, FOR_EACH, WHILE, DO_WHILE, };
    LoopType loopType;
};
Q_DECLARE_METATYPE(NodeInfo);

extern EnumStrMap sLoopTypeMapping;

bool operator==(const NodeInfo& lh, const NodeInfo& rh);

QDataStream& operator<<(QDataStream& out, const NodeInfo& data);
QDataStream& operator>>(QDataStream& in, NodeInfo& data);
QDataStream& operator<<(QDataStream& out, const NodeInfoList& data);
QDataStream& operator>>(QDataStream& in, NodeInfoList& data);

using  traverseNodeInfoFunc = std::function<bool(NodeInfo* node, NodeInfo* root, void* userData)>;
bool traverseNodeInfo(NodeInfo* node, NodeInfo* root, traverseNodeInfoFunc func, void* userData = nullptr);

QJsonObject nodeInfoToJson(const NodeInfo& node);
NodeInfo jsonToNodeInfo(const QJsonObject& obj);

//////////////////////////////////////////////////////////////////////////
// 变量定义
struct Variable
{
    QString name;
    QString type;
    int arrSize = 1;
    bool isPointer = false;
    QString value;
};
using VariableList = QList<Variable>;

QJsonArray varsToJson(const VariableList& vars);
VariableList jsonToVars(const QJsonArray& arr);

//////////////////////////////////////////////////////////////////////////
// BinCode定义
using BinCode = QPair<QString, int>;
using BinCodeList = QList<BinCode>;

QJsonArray binCodesToJson(const BinCodeList& vars);
BinCodeList jsonToBinCodes(const QJsonArray& arr);

//////////////////////////////////////////////////////////////////////////
// PinType定义，DEC编辑器使用
enum class PinType
{
    IO = 0,             /**< DIG板的IO通道,规格见手册 */
    BPMU = 1,           /**< DIG板上的PMU通道,规格见手册 */
    PPMU = 2,           /**< DIG板上的PPMU通道,规格见手册 */
    UR = 3,             /**< ECU板上的驱动IO,规格见手册 */
    BDPS = 4,           /**< DIG板上的DPS通道,规格见手册 */
    DPS64 = 5,          /**< DPS板上的DPS通道,规格见手册 */
    PMU = 6,            /**< PMU板上的PMU通道,规格见手册 */
    GND = 7,            /**< 测试芯片的接地Pin,规格见手册 */
    IN = 8,             /**< 测试芯片的输入Pin,规格见手册 */
    OUT = 9,            /**< 测试芯片的输出Pin,规格见手册 */
    VHH = 10,           /**< VHH高压输出引脚,规格见手册 */
    GPIO = 11,          /**< DIG、ECU板上的GPIO通道,规格见手册 */
    GPIO_DPS64 = 12,    /**< DPS板上的GPIO通道,规格见手册 */
};

extern EnumStrMap sPinTypeMapping;

// Pin定义，DEC编辑器使用
struct Pin
{
    QString name;
    QMap<int, int> siteIndexs; // <col index, site index>
    int dutIndex = 0;
    PinType type = PinType::IO;
};
Q_DECLARE_METATYPE(Pin)
using PinList = QList<Pin>;

QDataStream& operator<<(QDataStream& out, const Pin& data);
QDataStream& operator>>(QDataStream& in, Pin& data);
QDataStream& operator<<(QDataStream& out, const PinList& data);
QDataStream& operator>>(QDataStream& in, PinList& data);

QJsonArray pinListToJson(const PinList& vars);
PinList jsonToPinList(const QJsonArray& arr);

// PinGroup定义，DEC编辑器使用
using PinGroup = QPair<QString, QStringList>;
using PinGroupList = QList<PinGroup>;

QJsonArray pinGroupListToJson(const PinGroupList& vars);
PinGroupList jsonToPinGroupList(const QJsonArray& arr);

// TimeSet定义，DEC编辑器使用
struct TimeSet
{
    QString name;
    int interval;
};
using TimeSetList = QList<TimeSet>;

QJsonArray timeSetListToJson(const TimeSetList& vars);
TimeSetList jsonToTimeSetList(const QJsonArray& arr);

//////////////////////////////////////////////////////////////////////////
// 文件定义
struct File
{
    NodeInfo node;
    VariableList vars;
    BinCodeList hBins;
    BinCodeList sBins;
    PinList pinList;
    TimeSetList timeSetList;
    PinGroupList pinGroups;
    PinGroupList powerPinGroups;
    PinGroupList urPinGroups;
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

enum class IO
{
    IN, OUT
};

enum Direction
{
    Left, Top, Right, Bottom, VCenter, HCenter,
};

const auto DEFAULT_NODE_WIDTH = 300;
const auto DEFAULT_NODE_HEIGHT = 48;