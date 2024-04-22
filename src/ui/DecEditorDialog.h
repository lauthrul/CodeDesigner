#pragma once

#include <QDialog>
#include <QScopedPointer>
#include <QMimeData>
#include "ui_DecEditorDialog.h"
#include "core/DataManager.h"

//////////////////////////////////////////////////////////////////////////

class DecEditorDialog : public QDialog
{
    Q_OBJECT

public:
    DecEditorDialog(QWidget* parent = nullptr);
    ~DecEditorDialog();

public slots:
    void onSetSiteNums();
    void onAddPin();
    void onDelPin();
    void onPinListItemChanged(QTableWidgetItem* item);
    void onButtonBox(QAbstractButton* button);

private:
    void updatePinListTable();
    void initTimeSetList(const TimeSetList& list);
    void addTimeNameDef(const TimeSet& tm);
    void delTimeNameDef();
    TimeSetList timeSetList() const;

private:
    struct Private;
    QScopedPointer<Private> d;
    Ui::DecEditorDialog ui;
};
