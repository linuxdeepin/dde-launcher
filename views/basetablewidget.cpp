#include "basetablewidget.h"
#include <QFrame>
#include <QHeaderView>
#include <QTableWidgetItem>

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
}

void BaseTableWidget::setItemUnChecked(){
    for(int i = 0; i< rowCount(); i++){
        for (int j=0; j < colorCount(); j++){
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

BaseTableWidget::~BaseTableWidget()
{

}

