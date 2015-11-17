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
    setVerticalScrollMode(ScrollPerPixel);
    setEditTriggers(NoEditTriggers);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::NoFocus);
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
        if (!cellWidget(row, column)){
            emit signalManager->mouseReleased();
        }
    }else{
        emit signalManager->mouseReleased();
    }

    if (cellWidget(item->row(), item->column())){
        if (event->button() != Qt::RightButton){
            emit signalManager->Hide();
        }
    }else{
        emit signalManager->Hide();
    }


    QTableWidget::mouseReleaseEvent(event);
}

void BaseTableWidget::hideEvent(QHideEvent *event){
    clearHighlight();
    QTableWidget::hideEvent(event);
}

void BaseTableWidget::clearHighlight(){
    m_highlightRow = -1;
    m_highlightColumn = -1;
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
    if (m_highlightRow == -1 && m_highlightColumn == -1){
        firstHighlight();
    }else if (m_highlightRow == 0 && m_highlightColumn == 0){
        qDebug() << "first row";
    }else if (m_highlightRow >= 1 && m_highlightColumn >= 0){
        bool highlighted = false;
        for (int i= (m_highlightRow - 1); i>= 0; i--){
            for (int j=m_highlightColumn; j >=0; j--){
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
    if (m_highlightRow == -1 && m_highlightColumn == -1){
        firstHighlight();
    }else if (m_highlightRow < (rowCount() - 1) && m_highlightColumn >= 0){
        bool highlighted = false;
        for (int i= (m_highlightRow + 1); i< rowCount(); i++){
            for (int j=m_highlightColumn; j >=0; j--){
                if (cellWidget(i, j)){
                    QString className = QString(cellWidget(i, j)->metaObject()->className());
                    if (className == "AppItem"){
                        qDebug() << i << j;
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
    }else if (m_highlightRow == (rowCount() - 1) && m_highlightColumn >= 0){
        qDebug() << "last row";
    }
}

void BaseTableWidget::handleKeyLeftPressed(){
    if (m_highlightRow == -1 && m_highlightColumn == -1){
        firstHighlight();
    }else if (m_highlightRow == 0 && m_highlightColumn == 0){
        qDebug() << "first column";
    }else if (m_highlightColumn >= 0 && m_highlightRow >= 0){
        for (int i= (m_highlightColumn - 1); i>= 0; i--){
            qDebug() << cellWidget(m_highlightRow, i);
            if (cellWidget(m_highlightRow, i)){
                QString className = QString(cellWidget(m_highlightRow, i)->metaObject()->className());
                if (className == "AppItem"){
                    qDebug() << m_highlightRow << i;
                    hightlightItem(m_highlightRow, i);
                    return;
                }
            }
        }
        for (int i = (m_highlightRow -1); i >=0; i--){
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
    if (m_highlightRow == -1 && m_highlightColumn == -1){
        firstHighlight();
    }else if (m_highlightColumn <= (columnCount() - 1) && m_highlightRow >= 0){
        for (int j= (m_highlightColumn + 1); j< columnCount(); j++){
            if (cellWidget(m_highlightRow, j)){
                QString className = QString(cellWidget(m_highlightRow, j)->metaObject()->className());
                if (className == "AppItem"){
                    hightlightItem(m_highlightRow, j);
                    return;
                }
            }
        }

        for(int i=(m_highlightRow + 1); i< rowCount(); i++){
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
                setHighLight(i, 0);
                return;
            }
        }
    }
}

void BaseTableWidget::hightlightItem(int row, int column){
    QString lastName = QString(cellWidget(m_highlightRow, m_highlightColumn)->metaObject()->className());
    if (lastName == "AppItem"){
        AppItem* appItem = static_cast<AppItem*>(cellWidget(m_highlightRow, m_highlightColumn));
        appItem->getBorderButton()->setHighlight(false);
    }
    QString className = QString(cellWidget(row, column)->metaObject()->className());
    if (className== "AppItem"){
        setHighLight(row, column);
        if (row == 1){
            verticalScrollBar()->setValue(0);
        }else{
            scrollToItem(item(row, column), QAbstractItemView::PositionAtTop);
        }
    }
}

void BaseTableWidget::setHighLight(int hRow, int hColumn){
    AppItem* appItem = static_cast<AppItem*>(cellWidget(hRow, hColumn));
    appItem->getBorderButton()->setHighlight(true);
    m_highlightRow = hRow;
    m_highlightColumn = hColumn;
    emit currentAppItemChanged(appItem->getAppKey());
}

int BaseTableWidget::getHighLightRow() const {
    return m_highlightRow;
}

int BaseTableWidget::getHighLightColumn() const {
    return m_highlightColumn;
}

BaseTableWidget::~BaseTableWidget()
{

}

