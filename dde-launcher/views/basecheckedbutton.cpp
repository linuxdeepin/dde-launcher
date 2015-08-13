#include "basecheckedbutton.h"
#include <QEvent>


BaseCheckedButton::BaseCheckedButton(QWidget *parent) : QPushButton(parent)
{
    setCheckable(true);
    setFlat(true);
}


BaseCheckedButton::BaseCheckedButton(QString text, QWidget *parent):QPushButton(text, parent){
    setCheckable(true);
    setFlat(true);
}

void BaseCheckedButton::enterEvent(QEvent *event){
    emit mouseEnterToggled(true);
    QPushButton::enterEvent(event);
}

void BaseCheckedButton::leaveEvent(QEvent *event){
    emit mouseEnterToggled(false);
    QPushButton::leaveEvent(event);
}

BaseCheckedButton::~BaseCheckedButton()
{

}
