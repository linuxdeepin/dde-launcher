#include "apptablewidget.h"
#include "widgets/util.h"
#include "app/global.h"
#include "dbusinterface/dbustype.h"
#include "Logger.h"
#include "appitem.h"
#include "dbusinterface/dbustype.h"
#include "fwd.h"

#include <QTableWidgetItem>
#include <QWheelEvent>
#include <QScrollBar>


AppTableWidget::AppTableWidget(QWidget *parent) : BaseTableWidget(parent)
{
    setVerticalScrollMode(ScrollPerPixel);
    setObjectName("AppTableWidget");
    setStyleSheet(getQssFromFile(":/skin/qss/Tablewidget.qss"));
    initConnect();
}

void AppTableWidget::initConnect(){
    connect(signalManager, SIGNAL(sortModeChanged(int)), this, SLOT(showBySortedMode(int)));
    connect(signalManager, SIGNAL(itemInfosChanged(QMap<QString,ItemInfo>)), this, SLOT(setItemInfosMap(QMap<QString,ItemInfo>)));
}

void AppTableWidget::setGridParameter(int column, int girdWidth, int itemWidth){
    m_column = column;
    m_gridWidth = girdWidth;
    m_itemWidth = itemWidth;
    setColumnCount(m_column);
    for(int i=0; i< m_column; i++){
        setColumnWidth(i, m_gridWidth);
    }
}

void AppTableWidget::addItem(ItemInfo itemInfo, int index){
    int row = index / m_column;
    int column = index % m_column;


    QTableWidgetItem* item = new QTableWidgetItem();
    setItem(row, column, item);
    AppItem* appItem = new AppItem(itemInfo.url, itemInfo.icon, itemInfo.name);
    appItem->setFixedSize(m_gridWidth, m_gridWidth);
    setCellWidget(row, column, appItem);
}


void AppTableWidget::addItems(const QList<ItemInfo> &itemInfos){
    clearContents();
    setRowCount(itemInfos.length() / m_column + 1);
    for(int i=0; i< rowCount(); i++){
        setRowHeight(i, m_gridWidth);
    }
    for (int i=0; i< itemInfos.length(); i++) {
        addItem(itemInfos.at(i), i);
    }

    setItemUnChecked();
}

void AppTableWidget::setItemInfosMap(const QMap<QString, ItemInfo> &itemInfos){
    m_itemInfosMap = itemInfos;
}

void AppTableWidget::showBySortedMode(int mode){
    if (mode == 0){
        showbyName();
    }else if (mode == 3){
        showByInstalledTime();
    }else if (mode == 4){
        showByFrequency();
    }
}

void AppTableWidget::showbyName(){
    addItems(m_itemInfosMap.values());
}


void AppTableWidget::showByInstalledTime(){
    addItems(m_itemInfosMap.values());
}

void AppTableWidget::showByFrequency(){
    QList<ItemInfo> itemInfos;
    for(int i = m_itemInfosMap.values().count() - 1; i>0; i--){
        itemInfos.append(m_itemInfosMap.values().at(i));
    }
    addItems(itemInfos);
}

void AppTableWidget::wheelEvent(QWheelEvent *event){
    int value = verticalScrollBar()->value();
    int miniimun = verticalScrollBar()->minimum();
    int maximum = verticalScrollBar()->maximum();
    if (event->angleDelta().y() > 0){
        if (value >= miniimun){
            verticalScrollBar()->setValue(value - 40);
        }
    }else{
        if (value < maximum){
            verticalScrollBar()->setValue(value + 40);
        }
    }
}

AppTableWidget::~AppTableWidget()
{

}

