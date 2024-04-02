#pragma once

#include <QWidget>
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "FlowNode.h"

class PropertyWidget : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    explicit PropertyWidget(QWidget* parent = nullptr);
    ~PropertyWidget();

public slots:
    void currentFlowNodeChanged(const QString& uid);

private:
    struct Private;
    QSharedPointer<Private> d;
};
