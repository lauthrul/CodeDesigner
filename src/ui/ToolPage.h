#pragma once

#include <QWidget>

namespace Ui
{
    class ToolPage;
}

class ToolPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool expand READ isExpand WRITE setExpand)

public:
    explicit ToolPage(QWidget* parent = nullptr);
    ~ToolPage();

public:
    QString title() const;
    void setTitle(const QString& title);
    QWidget* widget();
    void setWidget(QWidget* widget);
    bool isExpand() const;
    void setExpand(bool b);
    void expand();
    void collapse();

signals:
    void toolPageExpandChanged(ToolPage* page, bool expand);

private:
    struct Private;
    QSharedPointer<Private> d;
    Ui::ToolPage* ui;
};
