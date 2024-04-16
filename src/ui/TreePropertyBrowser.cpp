#include "TreePropertyBrowser.h"
#include "core/Models.h"
#include "core/DataManager.h"
#include <QHeaderView>
#include <QDebug>
#include <QtWidgets/QApplication>
#include "ParamProperty.h"

//////////////////////////////////////////////////////////////////////////

const auto TEXT_BASIC = QT_TRANSLATE_NOOP("TreePropertyBrowser", "Basic");
const auto TEXT_NODETYPE = QT_TRANSLATE_NOOP("TreePropertyBrowser", "NodeType");
const auto TEXT_POSITION = QT_TRANSLATE_NOOP("TreePropertyBrowser", "Position");
const auto TEXT_PARAMS = QT_TRANSLATE_NOOP("TreePropertyBrowser", "Params");
const auto TEXT_FUNCRAW = QT_TRANSLATE_NOOP("TreePropertyBrowser", "FuncRaw");
const auto TEXT_FUNCRETTYPE = QT_TRANSLATE_NOOP("TreePropertyBrowser", "FuncRetType");
const auto TEXT_FUNCNAME = QT_TRANSLATE_NOOP("TreePropertyBrowser", "FuncName");
const auto TEXT_PARAMTYPE = QT_TRANSLATE_NOOP("TreePropertyBrowser", "ParamType");
const auto TEXT_PARAMNAME = QT_TRANSLATE_NOOP("TreePropertyBrowser", "ParamName");
const auto TEXT_PARAMVALUE = QT_TRANSLATE_NOOP("TreePropertyBrowser", "ParamValue");
const auto TEXT_CONDITION = QT_TRANSLATE_NOOP("TreePropertyBrowser", "Condition");
const auto TEXT_LOOPTYPE = QT_TRANSLATE_NOOP("TreePropertyBrowser", "LoopType");
const auto TEXT_CUSTOMCODE = QT_TRANSLATE_NOOP("TreePropertyBrowser", "CustomCode");

//////////////////////////////////////////////////////////////////////////

struct TreePropertyBrowser::Private
{
    QString m_uid;
    ParamPropertyManager* m_readonlyManager = nullptr;
    ParamPropertyManager* m_editableManager = nullptr;
    ParamEditorFactory* m_factory = nullptr;

    QtVariantProperty* findPropertyByName(ParamPropertyManager* manager, const QString& name)
    {
        for (auto prop : manager->properties())
        {
            if (prop->propertyName() == name)
                return dynamic_cast<QtVariantProperty*>(prop);
        }
        return (QtVariantProperty*)nullptr;
    }
};

TreePropertyBrowser::TreePropertyBrowser(QWidget* parent) :
    QtTreePropertyBrowser(parent),
    d(new TreePropertyBrowser::Private)
{
    setPropertiesWithoutValueMarked(true);
    setRootIsDecorated(false);
    setResizeMode(QtTreePropertyBrowser::Interactive);

    d->m_readonlyManager = new ParamPropertyManager(this);
    d->m_editableManager = new ParamPropertyManager(this);

    d->m_factory = new ParamEditorFactory(this);
    setFactoryForManager(d->m_editableManager, d->m_factory);

    connect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
    connect(DM_INST, &DataManager::nodeClicked, this, &TreePropertyBrowser::onNodeSelectionChanged);
    connect(DM_INST, &DataManager::nodePostionChanged, this, &TreePropertyBrowser::onNodePostionChanged);
}

TreePropertyBrowser::~TreePropertyBrowser()
{

}

void TreePropertyBrowser::onNodeSelectionChanged(const QString& uid)
{
    if (d->m_uid == uid) return;

    d->m_uid = uid;

    clear();
    d->m_editableManager->clear();
    d->m_readonlyManager->clear();

    auto* node = DM_INST->node().findChild(uid);
    if (node == nullptr) return;

    auto fnNewPropertyGroup = [&](QtVariantProperty * parent, ParamPropertyManager * manager,
                                  const QString & name)
    {
        QtVariantProperty* group = nullptr;
        if (parent == nullptr)
        {
            group = manager->addProperty(ParamPropertyManager::groupTypeId(), name);
            addProperty(group);
        }
        else
        {
            group = manager->addProperty(QVariant::String, name);
            parent->addSubProperty(group);
        }
        return group;
    };
    auto fnNewProperty = [&](QtVariantProperty * parent, ParamPropertyManager * manager,
                             int type, const QString & name, const QVariant& value = QVariant())
    {
        //disconnect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
        auto item = manager->addProperty(type, name);
        //connect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
        if (value != QVariant())
            item->setValue(value);
        parent->addSubProperty(item);
        return item;
    };

    // basic
    auto groupBasic = fnNewPropertyGroup(nullptr, d->m_readonlyManager, tr(TEXT_BASIC));
    fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr(TEXT_NODETYPE),
                  qApp->translate("Models", sNodeTypeMapping[node->type].toLocal8Bit()));
    fnNewProperty(groupBasic, d->m_editableManager, QVariant::PointF, tr(TEXT_POSITION), node->pos);

    // params
    auto groupParams = fnNewPropertyGroup(nullptr, d->m_readonlyManager, tr(TEXT_PARAMS));
    switch (node->type)
    {
        case NT_Function:
            {
                fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr(TEXT_FUNCRAW), node->function.raw);
                fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr(TEXT_FUNCRETTYPE), node->function.retType);
                fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr(TEXT_FUNCNAME), node->function.name);
                for (auto i = 0; i < node->function.params.size(); i++)
                {
                    const auto& param = node->function.params[i];
                    auto groupParam = fnNewPropertyGroup(groupParams, d->m_readonlyManager, param.name);
                    fnNewProperty(groupParam, d->m_readonlyManager, QVariant::String, tr(TEXT_PARAMTYPE), param.type);
                    fnNewProperty(groupParam, d->m_readonlyManager, QVariant::String, tr(TEXT_PARAMNAME), param.name);
                    QtVariantProperty* item;
                    if (node->function.name == "Test")
                    {
                        auto value = param.value.toString();
                        value = value.left(value.indexOf("-"));
                        item = fnNewProperty(groupParam, d->m_editableManager, StepPropertyType, tr(TEXT_PARAMVALUE), value);
                    }
                    else
                        item = fnNewProperty(groupParam, d->m_editableManager, ParamPropertyType, tr(TEXT_PARAMVALUE), param.value);
                    d->m_editableManager->setParam(item, { i, param });
                }
            }
            break;
        case NT_Condtion:
            {
                fnNewProperty(groupParams, d->m_editableManager, ParamPropertyType, tr(TEXT_CONDITION), node->condition);
            }
            break;
        case NT_Loop:
            {
                auto item = fnNewProperty(groupParams, d->m_editableManager, ParamPropertyManager::enumTypeId(), tr(TEXT_LOOPTYPE));
                QStringList enumNames;
                for (auto item : sLoopTypeMapping)
                    enumNames << item;
                disconnect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
                item->setAttribute(QLatin1String("enumNames"), enumNames);
                connect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
                item->setValue(node->loopType);
                fnNewProperty(groupParams, d->m_editableManager, ParamPropertyType, tr(TEXT_CONDITION), node->condition);
                //fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopInitial"), node->loopInitial);
                //fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopCondition"), node->loopCondition);
                //fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopIterator"), node->loopIterator);
            }
            break;
        case NT_CustomCode:
            {
                fnNewProperty(groupParams, d->m_editableManager, ParamPropertyType, tr(TEXT_CUSTOMCODE), node->condition);
            }
            break;
    }
}

void TreePropertyBrowser::onNodePostionChanged(const QString& uid, const QPointF& pos)
{
    if (d->m_uid != uid) return;
    auto prop = d->findPropertyByName(d->m_editableManager, tr(TEXT_POSITION));
    if (prop) prop->setValue(pos);
}

void TreePropertyBrowser::onValueChanged(QtProperty* property, const QVariant& val)
{
    qDebug() << property->propertyName() << val;

    auto* node = DM_INST->node().findChild(d->m_uid);
    if (node == nullptr) return;

    bool update = false;

    if (property->propertyName() == tr(TEXT_POSITION))
    {
        node->pos = val.toPointF();
        update = true;
    }
    else if (property->propertyName() == tr(TEXT_PARAMVALUE))
    {
        auto param = d->m_editableManager->param(property);
        if (param.first >= 0 && param.first < node->function.params.size())
        {
            if (node->function.name == "Test")
                node->function.params[param.first].value = val.toString() + "-" + property->valueText();
            else
                node->function.params[param.first].value = val;
            update = true;
        }
    }
    else if (property->propertyName() == tr(TEXT_CONDITION))
    {
        node->condition = val.toString();
        update = true;
    }
    else if (property->propertyName() == tr(TEXT_LOOPTYPE))
    {
        auto loopType = (NodeInfo::LoopType)val.toInt();
        //switch (loopType)
        //{
        //    case NodeInfo::WHILE:
        //    case NodeInfo::DO_WHILE:
        //    case NodeInfo::FOR_EACH:
        //        {
        //            auto prop = findProperty(d->m_editableManager, tr("LoopInitial"));
        //            if (prop) prop->setEnabled(false);

        //            prop = findProperty(d->m_editableManager, tr("LoopCondition"));
        //            if (prop) prop->setEnabled(true);

        //            prop = findProperty(d->m_editableManager, tr("LoopIterator"));
        //            if (prop) prop->setEnabled(false);
        //        }
        //        break;
        //    case NodeInfo::FOR:
        //        {
        //            auto prop = findProperty(d->m_editableManager, tr("LoopInitial"));
        //            if (prop) prop->setEnabled(true);

        //            prop = findProperty(d->m_editableManager, tr("LoopCondition"));
        //            if (prop) prop->setEnabled(true);

        //            prop = findProperty(d->m_editableManager, tr("LoopIterator"));
        //            if (prop) prop->setEnabled(true);
        //        }
        //        break;
        //}
        node->loopType = loopType;
        update = true;
    }
    //else if (property->propertyName() == tr("LoopInitial"))
    //{
    //    node->loopInitial = val.toString();
    //    update = true;
    //}
    //else if (property->propertyName() == tr("LoopCondition"))
    //{
    //    node->loopCondition = val.toString();
    //    update = true;
    //}
    //else if (property->propertyName() == tr("LoopIterator"))
    //{
    //    node->loopIterator = val.toString();
    //    update = true;
    //}
    else if (property->propertyName() == tr(TEXT_CUSTOMCODE))
    {
        node->condition = val.toString();
        update = true;
    }

    if (update) emit DM_INST->nodeValueChanged(*node);
}
