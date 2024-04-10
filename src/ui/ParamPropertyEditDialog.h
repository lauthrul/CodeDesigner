#pragma once

#include <QDialog>
#include "ui_ParamPropertyEditDialog.h"

class ParamPropertyEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParamPropertyEditDialog(QWidget* parent = nullptr);
    ~ParamPropertyEditDialog();

    void setTypeFilter(const QString& typeFilter);

    void setValue(const QString& value);
    QString value();

private:
    Ui::ParamPropertyEditDialog ui;
    QString m_typeFilter;
};
