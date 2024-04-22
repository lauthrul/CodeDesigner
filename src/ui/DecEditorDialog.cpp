#include "DecEditorDialog.h"
#include <QMessageBox>
#include <QComboBox>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QByteArray>
#include "core/DataManager.h"

//////////////////////////////////////////////////////////////////////////

struct DecEditorDialog::Private
{
    int siteNums = 1;
    PinList pinList;

    QStringList headers() const
    {
        QStringList list = { tr("Pin") };
        for (int i = 0; i < siteNums; i++)
            list << QString("%1%2").arg(tr("Site")).arg(i + 1);
        list << tr("DUT") << tr("PinType");
        return list;
    };
};

QTableWidgetItem* createTableItem(QTableWidget* table, int row, int col, const QString& text)
{
    QTableWidgetItem* item = new QTableWidgetItem(text);
    table->setItem(row, col, item);
    return item;
};

std::list<int> selectRows(QTableWidget* table)
{
    std::map<int, bool> rows;
    auto items = table->selectedItems();
    for (auto item : items)
        rows[item->row()] = false;
    std::list<int> ret;
    for (auto row : rows)
        ret.push_back(row.first);
    return ret;
}

//////////////////////////////////////////////////////////////////////////

DecEditorDialog::DecEditorDialog(QWidget* parent)
    : QDialog(parent), d(new Private)
{
    ui.setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    d->pinList = DM_INST->file().pinList;

    if (!d->pinList.isEmpty())
    {
        d->siteNums = d->pinList[0].siteIndexs.size();
        ui.spinBoxSiteNums->setValue(d->siteNums);
    }

    ui.tablePinList->setData(&d->pinList);
    updatePinListTable();

    initTimeSetList(DM_INST->file().timeSetList);

    ui.widgetPinGroup->setTitle("PIN_GROUP");
    ui.widgetURPinGroup->setTitle("UR_PIN_GROUP");
    ui.widgetPowerPinGroup->setTitle("POWER_PIN_GROUP");
    ui.widgetPinGroup->setData(DM_INST->file().pinGroups);
    ui.widgetPowerPinGroup->setData(DM_INST->file().powerPinGroups);
    ui.widgetURPinGroup->setData(DM_INST->file().urPinGroups);

    connect(ui.btnSetSiteNums, &QPushButton::clicked, this, &DecEditorDialog::onSetSiteNums);
    connect(ui.btnAddPin, &QPushButton::clicked, this, &DecEditorDialog::onAddPin);
    connect(ui.btnDelPin, &QPushButton::clicked, this, &DecEditorDialog::onDelPin);
    connect(ui.tablePinList, &QTableWidget::itemChanged, this, &DecEditorDialog::onPinListItemChanged);
    connect(ui.btnAddTimeNameDef, &QPushButton::clicked, this, [&]() { addTimeNameDef(TimeSet()); });
    connect(ui.btnDelTimeNameDef, &QPushButton::clicked, this, &DecEditorDialog::delTimeNameDef);
    connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &DecEditorDialog::onButtonBox);
}

DecEditorDialog::~DecEditorDialog()
{

}

void DecEditorDialog::onSetSiteNums()
{
    auto siteNums = ui.spinBoxSiteNums->value();
    if (siteNums == d->siteNums) return;
    if (siteNums < d->siteNums
            && QMessageBox::warning(
                this, "",
                tr("The reduction in the site numbers will result in the data loss of Pin List, are you sure you want to do this?"),
                QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
    {
        return;
    }

    if (siteNums < d->siteNums) // reduce
    {
        auto diff = d->siteNums - siteNums;
        for (auto& pin : d->pinList)
        {
            for (int i = 0; i < diff; i++)
                pin.siteIndexs.remove(pin.siteIndexs.lastKey());
        }
    }
    else // increase
    {
        auto diff = siteNums - d->siteNums;
        for (auto& pin : d->pinList)
        {
            for (int i = 0; i < diff; i++)
                pin.siteIndexs[d->siteNums + i] = 0;
        }
    }

    d->siteNums = siteNums;

    updatePinListTable();
}

void DecEditorDialog::onAddPin()
{
    Pin pin;
    for (int i = 0; i < d->siteNums; i++)
        pin.siteIndexs[i] = 0;
    d->pinList.push_back(pin);
    updatePinListTable();
}

void DecEditorDialog::onDelPin()
{
    auto row = ui.tablePinList->currentRow();
    if (row < 0 || row >= d->pinList.size()) return;
    d->pinList.removeAt(row);
    ui.tablePinList->removeRow(row);
}

void DecEditorDialog::onPinListItemChanged(QTableWidgetItem* item)
{
    auto row = item->row();
    auto col = item->column();
    if (row < 0 || row >= d->pinList.size()) return;
    auto& pin = d->pinList[row];
    auto columns = ui.tablePinList->columnCount();
    if (col == 0)
        pin.name = item->text();
    else if (col >= 1 && col < columns - 2)
        pin.siteIndexs[col - 1] = item->text().toInt();
    else if (col == columns - 2)
        pin.dutIndex = item->text().toInt();
    else if (col == columns - 1)
        pin.type = (PinType)item->text().toInt();
}

void DecEditorDialog::onButtonBox(QAbstractButton* button)
{
    if (button == ui.buttonBox->buttons()[0])
    {
        DM_INST->file().pinList = d->pinList;
        DM_INST->file().timeSetList = timeSetList();
        DM_INST->file().pinGroups = ui.widgetPinGroup->data();
        DM_INST->file().powerPinGroups = ui.widgetPowerPinGroup->data();
        DM_INST->file().urPinGroups = ui.widgetURPinGroup->data();
        accept();
    }
    else
    {
        reject();
    }
}

void DecEditorDialog::updatePinListTable()
{
    ui.tablePinList->clear();

    auto headers = d->headers();
    ui.tablePinList->setColumnCount(headers.size());
    ui.tablePinList->setHorizontalHeaderLabels(headers);

    ui.tablePinList->setRowCount(d->pinList.size());
    for (auto i = 0; i < d->pinList.size(); i++)
    {
        const auto& pin = d->pinList[i];
        auto col = 0;
        createTableItem(ui.tablePinList, i, col++, pin.name);
        for (const auto& siteIndex : pin.siteIndexs)
            createTableItem(ui.tablePinList, i, col++, QString::number(siteIndex));
        createTableItem(ui.tablePinList, i, col++, QString::number(pin.dutIndex));

        QComboBox* cmb = new QComboBox(ui.tablePinList);
        for (const auto& v : sPinTypeMapping.toStdMap())
            cmb->addItem(v.second, v.first);
        cmb->setCurrentIndex((int)pin.type);
        cmb->setProperty("row", i);
        ui.tablePinList->setCellWidget(i, col, cmb);
        connect(cmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [ = ](int index)
        {
            auto row = cmb->property("row").toInt();
            d->pinList[row].type = (PinType)index;
        });
    }
}

void DecEditorDialog::initTimeSetList(const TimeSetList& list)
{
    for (const auto& v : list)
        addTimeNameDef(v);
}

void DecEditorDialog::addTimeNameDef(const TimeSet& tm)
{
    auto row = ui.tableTimeNameDef->rowCount();
    ui.tableTimeNameDef->setRowCount(row + 1);
    if (!tm.name.isEmpty())
    {
        createTableItem(ui.tableTimeNameDef, row, 0, tm.name);
        createTableItem(ui.tableTimeNameDef, row, 1, QString::number(tm.interval));
    }
    else
    {
        createTableItem(ui.tableTimeNameDef, row, 0, QString("TM%1").arg(row + 1));
        createTableItem(ui.tableTimeNameDef, row, 1, QString::number(row + 1));
    }
}

void DecEditorDialog::delTimeNameDef()
{
    auto rows = selectRows(ui.tableTimeNameDef);
    for (auto it = rows.rbegin(); it != rows.rend(); it++)
        ui.tableTimeNameDef->removeRow(*it);
}

TimeSetList DecEditorDialog::timeSetList() const
{
    TimeSetList list;
    for (int i = 0; i < ui.tableTimeNameDef->rowCount(); i++)
    {
        TimeSet tm;
        auto item = ui.tableTimeNameDef->item(i, 0);
        if (item) tm.name = item->text();
        item = ui.tableTimeNameDef->item(i, 1);
        if (item) tm.interval = item->text().toInt();
        list << tm;
    }
    return list;
}
