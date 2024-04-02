#include "ToolPage.h"
#include "ui_ToolPage.h"

struct ToolPage::Private
{
    bool m_expand = false;
    QWidget* m_widget = nullptr;
};

ToolPage::ToolPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ToolPage),
    d(new ToolPage::Private)
{
    ui->setupUi(this);
    ui->widgetContent->setAttribute(Qt::WA_StyledBackground);
    connect(ui->tbnFold, &QToolButton::clicked, this, [&]()
    {
        if (d->m_expand) collapse();
        else expand();
        emit toolPageExpandChanged(this, d->m_expand);
    });
}

ToolPage::~ToolPage()
{
    delete ui;
}

QString ToolPage::title() const
{
    return ui->lblTitle->text();
}

void ToolPage::setTitle(const QString& title)
{
    ui->lblTitle->setText(title);
}

QWidget* ToolPage::widget()
{
    return d->m_widget;
}

void ToolPage::setWidget(QWidget* widget)
{
    d->m_widget = widget;
    ui->verticalLayoutContent->addWidget(widget);
}

bool ToolPage::isExpand() const
{
    return d->m_expand;
}

void ToolPage::setExpand(bool b)
{
    if (b) expand();
    else collapse();
}

void ToolPage::expand()
{
    d->m_expand = true;
    ui->widgetContent->show();
    ui->tbnFold->setArrowType(Qt::ArrowType::DownArrow);
}

void ToolPage::collapse()
{
    d->m_expand = false;
    ui->widgetContent->hide();
    ui->tbnFold->setArrowType(Qt::ArrowType::LeftArrow);
}
