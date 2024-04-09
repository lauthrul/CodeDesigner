#include "PropertyWidget.h"
#include "core/Models.h"
#include <QHeaderView>
#include <QDebug>
#include "core/DataManager.h"

struct PropertyWidget::Private
{
    QtVariantPropertyManager* m_readonlyManager = nullptr;
    QtVariantPropertyManager* m_editableManager = nullptr;
    QtVariantEditorFactory* m_factory = nullptr;
};

PropertyWidget::PropertyWidget(QWidget* parent) :
    QtTreePropertyBrowser(parent),
    d(new PropertyWidget::Private)
{
    setPropertiesWithoutValueMarked(true);
    setRootIsDecorated(false);
    setResizeMode(QtTreePropertyBrowser::Interactive);

    d->m_readonlyManager = new QtVariantPropertyManager(this);
    d->m_editableManager = new QtVariantPropertyManager(this);

    d->m_factory = new QtVariantEditorFactory(this);
    setFactoryForManager(d->m_editableManager, d->m_factory);

    connect(d->m_editableManager, &QtVariantPropertyManager::valueChanged, this, &PropertyWidget::onValueChanged);
}

PropertyWidget::~PropertyWidget()
{

}

void PropertyWidget::onNodeSelectionChanged(const QString& uid)
{
    clear();

    auto* node = DM_INST->node().findChild(uid);
    if (node == nullptr) return;

    auto fnNewPropertyGroup = [&](QtVariantProperty * parent, QtVariantPropertyManager * manager,
                                  const QString & name)
    {
        QtVariantProperty* group = nullptr;
        if (parent == nullptr)
        {
            group = manager->addProperty(QtVariantPropertyManager::groupTypeId(), name);
            addProperty(group);
        }
        else
        {
            group = manager->addProperty(QVariant::String, name);
            parent->addSubProperty(group);
        }
        return group;
    };
    auto fnNewProperty = [](QtVariantProperty * parent, QtVariantPropertyManager * manager,
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
                for (const auto& param : node->function.params)
                {
                    auto groupParam = fnNewPropertyGroup(groupParams, d->m_readonlyManager, param.name);
                    fnNewProperty(groupParam, d->m_readonlyManager, QVariant::String, tr("ParamType"), param.type);
                    fnNewProperty(groupParam, d->m_readonlyManager, QVariant::String, tr("ParamName"), param.name);
                    QMap<QString, QVariant::Type> mapping =
                    {
                        {"bool", QVariant::Bool},
                        {"int", QVariant::Int},
                        {"unsigned int", QVariant::UInt},
                        {"char*", QVariant::String},
                        {"const char*", QVariant::String},
                        {"double", QVariant::Double},
                    };
                    if (mapping.contains(param.type))
                        fnNewProperty(groupParam, d->m_editableManager, mapping[param.type], tr("ParamValue"), param.value);
                }
            }
            break;
        case NT_Condtion:
            {
                fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("Condition"), node->condition);
            }
            break;
        case NT_Loop:
            {
                auto item = fnNewProperty(groupParams, d->m_editableManager, QtVariantPropertyManager::enumTypeId(), tr("LoopType"));
                QStringList enumNames;
                for (auto item : sLoopTypeMapping)
                    enumNames << item;
                item->setAttribute(QLatin1String("enumNames"), enumNames);
                item->setValue(node->loopType);
                fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopInitial"), node->loopInitial);
                fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopCondition"), node->loopCondition);
                fnNewProperty(groupParams, d->m_editableManager, QVariant::String, tr("LoopIterator"), node->loopIterator);
            }
            break;
    }
}

void PropertyWidget::onValueChanged(QtProperty* property, const QVariant& val)
{
    auto findProperty = [](QtVariantPropertyManager * manager, const QString & name)
    {
        for (auto prop : manager->properties())
        {
            if (prop->propertyName() == name)
                return prop;
        }
        return (QtProperty*)nullptr;
    };

    qDebug() << property->propertyName() << val;
    if (property->propertyName() == tr("LoopType"))
    {
        auto loopType = (NodeInfo::LoopType)val.toInt();
        switch (loopType)
        {
            case NodeInfo::WHILE:
            case NodeInfo::DO_WHILE:
            case NodeInfo::FOR_EACH:
                {
                    auto prop = findProperty(d->m_editableManager, tr("LoopInitial"));
                    if (prop) prop->setEnabled(false);

                    prop = findProperty(d->m_editableManager, tr("LoopCondition"));
                    if (prop) prop->setEnabled(true);

                    prop = findProperty(d->m_editableManager, tr("LoopIterator"));
                    if (prop) prop->setEnabled(false);
                }
                break;
            case NodeInfo::FOR:
                {
                    auto prop = findProperty(d->m_editableManager, tr("LoopInitial"));
                    if (prop) prop->setEnabled(true);

                    prop = findProperty(d->m_editableManager, tr("LoopCondition"));
                    if (prop) prop->setEnabled(true);

                    prop = findProperty(d->m_editableManager, tr("LoopIterator"));
                    if (prop) prop->setEnabled(true);
                }
                break;
        }
    }
}
