#include "dlineedit.h"
#include "app/global.h"

DLineEdit::DLineEdit(QWidget *parent) :
    QLineEdit(parent)
{

}


void DLineEdit::keyPressEvent(QKeyEvent *event){
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Left){
        emit signalManager->keyDirectionPressed(Qt::Key_Left);
    }else if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Right){
        emit signalManager->keyDirectionPressed(Qt::Key_Right);
    }
    QLineEdit::keyPressEvent(event);
}
