#include "basetablewidget.h"
#include "app/global.h"
#include "widgets/util.h"
#include <QFrame>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMouseEvent>
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
    QTableWidget::mouseReleaseEvent(event);
}

BaseTableWidget::~BaseTableWidget()
{

}

