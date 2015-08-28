#include "basetablewidget.h"
#include "app/global.h"
#include "widgets/util.h"
#include <QFrame>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMouseEvent>
#include <QHideEvent>
#include "borderbutton.h"
#include "appitem.h"

BaseTableWidget::BaseTableWidget(QWidget *parent) : QTableWidget(parent)
{
    setSelectionBehavior(SelectItems);
    setSelectionMode(NoSelection);
    setDragDropMode(NoDragDrop);
    setVerticalScrollMode(ScrollPerItem);
    setEditTriggers(NoEditTriggers);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
//    setMouseTracking(true);
    verticalHeader()->hide();
    horizontalHeader()->hide();
    setShowGrid(false);
    setStyleSheet(getQssFromFile(":/qss/skin/qss/Tablewidget.qss"));
}

void BaseTableWidget::setItemUnChecked(){
    for(int i = 0; i< rowCount(); i++){
        for (int j=0; j < columnCount(); j++){
            if (item(i, j)){
                item(i, j)->setFlags(Qt::NoItemFlags);
            }else{
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setFlags(Qt::NoItemFlags);
                setItem(i, j, item);
            }
        }
    }
}

void BaseTableWidget::mouseReleaseEvent(QMouseEvent *event){
    QTableWidgetItem* item = itemAt(event->pos());
    if (item){
        int row = item->row();
        int column = item->column();
        if (cellWidget(row, column)){

        }else{
            emit signalManager->mouseReleased();
        }
    }else{
        emit signalManager->mouseReleased();
    }
    emit signalManager->Hide();
    QTableWidget::mouseReleaseEvent(event);
}

void BaseTableWidget::hideEvent(QHideEvent *event){
    clearHighlight();
    QTableWidget::hideEvent(event);
}

void BaseTableWidget::clearHighlight(){
    p_highlightRow = -1;
    p_highlightColumn = -1;
}

void BaseTableWidget::handleDirectionKeyPressed(Qt::Key key){
    if (isVisible()){
        if (key == Qt::Key_Up){
            handleKeyUpPressed();
        }else if (key == Qt::Key_Down) {
            handleKeyDownPressed();
        }else if (key == Qt::Key_Left){
            handleKeyLeftPressed();
        }else if (key == Qt::Key_Right){
            handleKeyRightPressed();
        }
    }
}

void BaseTableWidget::handleKeyUpPressed(){
    if (p_highlightRow == -1 && p_highlightColumn == -1){
        firstHighlight();
    }else if (p_highlightRow == 0 && p_highlightColumn == 0){
        qDebug() << "first row";
    }else if (p_highlightRow >= 1 && p_highlightColumn >= 0){
        bool highlighted = false;
        for (int i= (p_highlightRow - 1); i>= 0; i--){
            for (int j=p_highlightColumn; j >=0; j--){
                if (cellWidget(i, j)){
                    QString className = QString(cellWidget(i, j)->metaObject()->className());
                    if (className == "AppItem"){
                        hightlightItem(i, j);
                        highlighted = true;
                        break;
                    }
                }
            }
            if (highlighted){
                break;
            }
        }
    }
}

void BaseTableWidget::handleKeyDownPressed(){
    if (p_highlightRow == -1 && p_highlightColumn == -1){
        firstHighlight();
    }else if (p_highlightRow < (rowCount() - 1) && p_highlightColumn >= 0){
        bool highlighted = false;
        for (int i= (p_highlightRow + 1); i< rowCount(); i++){
            for (int j=p_highlightColumn; j >=0; j--){
                if (cellWidget(i, j)){
                    QString className = QString(cellWidget(i, j)->metaObject()->className());
                    if (className == "AppItem"){
                        hightlightItem(i, j);
                        highlighted = true;
                        break;
                    }
                }
            }
            if (highlighted){
                break;
            }
        }
    }else if (p_highlightRow == (rowCount() - 1) && p_highlightColumn >= 0){
        qDebug() << "last row";
    }
}

void BaseTableWidget::handleKeyLeftPressed(){
    if (p_highlightRow == -1 && p_highlightColumn == -1){
        firstHighlight();
    }else if (p_highlightRow == 0 && p_highlightColumn == 0){
        qDebug() << "first column";
    }else if (p_highlightColumn >= 0 && p_highlightRow >= 0){
        for (int i= (p_highlightColumn - 1); i>= 0; i--){
            qDebug() << cellWidget(p_highlightRow, i);
            if (cellWidget(p_highlightRow, i)){
                QString className = QString(cellWidget(p_highlightRow, i)->metaObject()->className());
                if (className == "AppItem"){
                    qDebug() << p_highlightRow << i;
                    hightlightItem(p_highlightRow, i);
                    return;
                }
            }
        }
        for (int i = (p_highlightRow -1); i >0; i--){
            for (int j= (columnCount() - 1); j > 0; j--){
                if (cellWidget(i, j)){
                    QString className = QString(cellWidget(i, j)->metaObject()->className());
                    if (className == "AppItem"){
                        hightlightItem(i, j);
                        return;
                    }
                }
            }
        }
    }
}

void BaseTableWidget::handleKeyRightPressed(){
    if (p_highlightRow == -1 && p_highlightColumn == -1){
        firstHighlight();
    }else if (p_highlightColumn <= (columnCount() - 1) && p_highlightRow >= 0){
        for (int j= (p_highlightColumn + 1); j< columnCount(); j++){
            if (cellWidget(p_highlightRow, j)){
                QString className = QString(cellWidget(p_highlightRow, j)->metaObject()->className());
                if (className == "AppItem"){
                    hightlightItem(p_highlightRow, j);
                    return;
                }
            }
        }

        for(int i=(p_highlightRow + 1); i< rowCount(); i++){
            for (int j= 0; j< columnCount(); j++){
                if (cellWidget(i, j)){
                    QString className = QString(cellWidget(i, j)->metaObject()->className());
                    if (className == "AppItem"){
                        hightlightItem(i, j);
                        return;
                    }
                }
            }
        }
    }
}

void BaseTableWidget::firstHighlight(){
    for(int i=0; i< rowCount(); i++){
        if (cellWidget(i, 0)){
            QString className = QString(cellWidget(i, 0)->metaObject()->className());
            if (className == "AppItem"){
                AppItem* appItem = static_cast<AppItem*>(cellWidget(i, 0));
                appItem->getBorderButton()->setHighlight(true);
                p_highlightRow = i;
                p_highlightColumn = 0;
                return;
            }
        }
    }
}

void BaseTableWidget::hightlightItem(int row, int column){
    QString lastName = QString(cellWidget(p_highlightRow, p_highlightColumn)->metaObject()->className());
    if (lastName == "AppItem"){
        AppItem* appItem = static_cast<AppItem*>(cellWidget(p_highlightRow, p_highlightColumn));
        appItem->getBorderButton()->setHighlight(false);
    }
    QString className = QString(cellWidget(row, column)->metaObject()->className());
    if (className== "AppItem"){
        AppItem* appItem = static_cast<AppItem*>(cellWidget(row, column));
        appItem->getBorderButton()->setHighlight(true);
        p_highlightRow = row;
        p_highlightColumn = column;
        scrollToItem(item(p_highlightRow, p_highlightColumn));
    }
}


BaseTableWidget::~BaseTableWidget()
{

}

