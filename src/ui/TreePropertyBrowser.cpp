#include "TreePropertyBrowser.h"
#include "core/Models.h"
#include "core/DataManager.h"
#include <QHeaderView>
#include <QDebug>
#include "ParamProperty.h"

struct TreePropertyBrowser::Private
{
    QString m_uid;
    ParamPropertyManager* m_readonlyManager = nullptr;
    ParamPropertyManager* m_editableManager = nullptr;
    ParamEditorFactory* m_factory = nullptr;
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
}

TreePropertyBrowser::~TreePropertyBrowser()
{

}

void TreePropertyBrowser::onNodeSelectionChanged(const QString& uid)
{
    clear();

    auto* node = DM_INST->node().findChild(uid);
    if (node == nullptr) return;

    d->m_uid = uid;

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
    auto fnNewProperty = [](QtVariantProperty * parent, ParamPropertyManager * manager,
                            int type, const QString & name, const QVariant& value = QVariant())
    {
        auto item = manager->addProperty(type, name);
        if (value != QVariant())
            item->setValue(value);
        parent->addSubProperty(item);
        return item;
    };

    // basic
    auto groupBasic = fnNewPropertyGroup(nullptr, d->m_readonlyManager, tr("Basic"));
    fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr("NodeType"), tr(sNodeTypeMapping[node->type].toLocal8Bit()));

    // params
    auto groupParams = fnNewPropertyGroup(nullptr, d->m_readonlyManager, tr("Params"));
    switch (node->type)
    {
        case NT_Function:
            {
                fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr("FuncRaw"), node->function.raw);
                fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr("FuncRetType"), node->function.retType);
                fnNewProperty(groupBasic, d->m_readonlyManager, QVariant::String, tr("FuncName"), node->function.name);
                for (auto i = 0; i < node->function.params.size(); i++)
                {
                    const auto& param = node->function.params[i];
                    auto groupParam = fnNewPropertyGroup(groupParams, d->m_readonlyManager, param.name);
                    fnNewProperty(groupParam, d->m_readonlyManager, QVariant::String, tr("ParamType"), param.type);
                    fnNewProperty(groupParam, d->m_readonlyManager, QVariant::String, tr("ParamName"), param.name);
                    auto item = fnNewProperty(groupParam, d->m_editableManager, ParamPropertyType, tr("ParamValue"), param.value);
                    d->m_editableManager->setParam(item, { i, param });
                }
            }
            break;
        case NT_Condtion:
            {
                fnNewProperty(groupParams, d->m_editableManager, ParamPropertyType, tr("Condition"), node->condition);
            }
            break;
        case NT_Loop:
            {
                auto item = fnNewProperty(groupParams, d->m_editableManager, ParamPropertyManager::enumTypeId(), tr("LoopType"));
                QStringList enumNames;
                for (auto item : sLoopTypeMapping)
                    enumNames << item;
                disconnect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
                item->setAttribute(QLatin1String("enumNames"), enumNames);
                connect(d->m_editableManager, &ParamPropertyManager::valueChanged, this, &TreePropertyBrowser::onValueChanged);
                item->setValue(node->loopType);
                fnNewProperty(groupParams, d->m_editableManager, ParamPropertyType, tr("Condition"), node->condition);
                //fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopInitial"), node->loopInitial);
                //fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopCondition"), node->loopCondition);
                //fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopIterator"), node->loopIterator);
            }
            break;
    }
}

void TreePropertyBrowser::onValueChanged(QtProperty* property, const QVariant& val)
{
    auto findProperty = [](ParamPropertyManager * manager, const QString & name)
    {
        for (auto prop : manager->properties())
        {
            if (prop->propertyName() == name)
                return prop;
        }
        return (QtProperty*)nullptr;
    };

    qDebug() << property->propertyName() << val;

    auto* node = DM_INST->node().findChild(d->m_uid);
    if (node == nullptr) return;

    bool update = false;

    if (property->propertyName() == tr("ParamValue"))
    {
        auto param = d->m_editableManager->param(property);
        if (param.first >= 0 && param.first < node->function.params.size())
        {
            node->function.params[param.first].value = val;
            update = true;
        }
    }
    else if (property->propertyName() == tr("Condition"))
    {
        node->condition = val.toString();
        update = true;
    }
    else if (property->propertyName() == tr("LoopType"))
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

    if (update) emit DM_INST->nodeValueChanged(*node);
}
