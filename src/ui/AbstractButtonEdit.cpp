#include "AbstractButtonEdit.h"
#include <QLineEdit>
#include <QHBoxLayout>

AbstractButtonEdit::AbstractButtonEdit(QWidget* parent) :
    QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
}

AbstractButtonEdit::~AbstractButtonEdit()
{

}
