#include "categorytablewidget.h"
#include "widgets/util.h"
#include "app/global.h"
#include "categoryitem.h"
#include "Logger.h"
#include <QWheelEvent>
#include <QHeaderView>
#include <QApplication>
#include <QDesktopWidget>

CategoryTableWidget::CategoryTableWidget(QWidget *parent) : BaseTableWidget(parent)
{
    setObjectName("CategoryTableWidget");
    setStyleSheet(getQssFromFile(":/skin/qss/Tablewidget.qss"));
    initConnect();
}

void CategoryTableWidget::initConnect(){
    connect(signalManager, SIGNAL(categoryInfosChanged(CategoryInfoList)), this, SLOT(setCategoryInfoList(CategoryInfoList)));
    connect(signalManager, SIGNAL(itemInfosChanged(QMap<QString,ItemInfo>)), this, SLOT(setItemInfosMap(QMap<QString,ItemInfo>)));
    connect(signalManager, SIGNAL(navigationButtonClicked(QString)), this, SLOT(scrollToCategory(QString)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(handleScrollBarValueChanged(int)));
}

void CategoryTableWidget::setGridParameter(int column, int girdWidth, int itemWidth){
    m_column = column;
    m_gridWidth = girdWidth;
    m_itemWidth = itemWidth;
    setColumnCount(m_column);
    for(int i=0; i< m_column; i++){
        setColumnWidth(i, m_gridWidth);
    }

    int desktopHeight =  qApp->desktop()->screenGeometry().height();
    int bottomMargin = qApp->desktop()->screenGeometry().height() - qApp->desktop()->availableGeometry().height();
    setFixedHeight(desktopHeight - 2* bottomMargin);

}

void CategoryTableWidget::setItemInfosMap(const QMap<QString, ItemInfo> &itemInfos){
    m_itemInfosMap = itemInfos;
}

void CategoryTableWidget::setCategoryInfoList(const CategoryInfoList &categoryInfoList){
    m_categoryInfoList = categoryInfoList;
    for(int i = 0; i<= 9; i++ ) {
        foreach (CategoryInfo info, categoryInfoList) {
            if (info.id == i){
                m_sortedCategoryInfoList.append(info);
            }
        }
    }
    foreach (CategoryInfo info, categoryInfoList) {
        if (info.id == -2){
            m_sortedCategoryInfoList.append(info);
        }
    }

    foreach (CategoryInfo info, m_sortedCategoryInfoList) {
        if (info.items.count() == 0){
            m_hideKeys.append(info.key);
        }
    }
    emit signalManager->hideNavigationButtonByKeys(m_hideKeys);

    addItems(m_sortedCategoryInfoList);

    setItemUnChecked();
}

void CategoryTableWidget::addCategoryItem(int row, QString key){
     QTableWidgetItem* item = new QTableWidgetItem();
     item->setFlags(Qt::NoItemFlags);
     setSpan(row, 0, 1, m_column);
     setRowHeight(row, 40);
     setItem(row, 0, item);

     CategoryItem* categoryItem = new CategoryItem(key);
     categoryItem->setFixedSize(m_gridWidth * m_column, 40);
     setCellWidget(row, 0, categoryItem);
     m_categoryItems.insert(key, categoryItem);
}

void CategoryTableWidget::addItems(int row, QString categoryKey, QStringList appKeys){
    int startRow = row;
    int endRow;
    if (appKeys.count() % m_column == 0){
        endRow = appKeys.count() / m_column + row;
    }else{
        endRow = appKeys.count() / m_column + row + 1;
    }
    for(int i=startRow; i< endRow; i++){
       insertRow(i);
       setRowHeight(i, m_gridWidth);
    }
    foreach (QString appKey, appKeys) {
       int _row = startRow + appKeys.indexOf(appKey) / m_column;
       int column = appKeys.indexOf(appKey) % m_column;

       if (m_itemInfosMap.contains(appKey)){
            const ItemInfo& itemInfo = m_itemInfosMap.value(appKey);

            AppItem* appItem = new AppItem(itemInfo.url, itemInfo.icon, itemInfo.name);
            appItem->setFixedSize(m_gridWidth, m_gridWidth);
            setCellWidget(_row, column, appItem);
       }else{
           LOG_INFO() << appKey << m_itemInfosMap.keys();
       }
   }

   if (categoryKey == "others"){
       int desktopHeight =  qApp->desktop()->screenGeometry().height();
       int bottomMargin = qApp->desktop()->screenGeometry().height() - qApp->desktop()->availableGeometry().height();
       int othersCategoryHeight = categoryItemHeight + m_gridWidth * (endRow - startRow);
       if (othersCategoryHeight <  desktopHeight - 2 * bottomMargin){
           insertRow(endRow);
           setRowHeight(endRow, height() - othersCategoryHeight);
       }
   }
}

void CategoryTableWidget::addItems(const CategoryInfoList &categoryInfoList){
    clearContents();
    int row = 0;
    foreach (CategoryInfo info, categoryInfoList) {
        if(info.items.count() > 0){
            insertRow(rowCount());
            row = rowCount() - 1;
            addCategoryItem(row, info.key);
            addItems(rowCount(), info.key,  info.items);
        }
    }
}

void CategoryTableWidget::scrollToCategory(QString key){
    if (m_categoryItems.contains(key)){
        for (int i=0; i< rowCount() ; i++){
            if (m_categoryItems.value(key) == static_cast<CategoryItem*>(cellWidget(i, 0))){
                scrollToItem(item(i, 0), PositionAtTop);
            }
        }
    }
}

void CategoryTableWidget::handleScrollBarValueChanged(int value){
    while (true) {
        if (m_categoryItems.values().contains(static_cast<CategoryItem*>(cellWidget(value, 0)))){
            foreach (QString key, m_categoryItems.keys()) {
                if (m_categoryItems.value(key)  == cellWidget(value, 0)){
                    LOG_INFO() << key;
                    emit signalManager->checkNavigationButtonByKey(key);
                    break;
                }
            }
            break;
        }
        value -= 1;
        if (value < 0){
            break;
        }
    }
}

void CategoryTableWidget::wheelEvent(QWheelEvent *event){
    int value = verticalScrollBar()->value();
    int miniimun = verticalScrollBar()->minimum();
    int maximum = verticalScrollBar()->maximum();
    if (event->angleDelta().y() > 0){
        if (value >= miniimun){
            verticalScrollBar()->setValue(value - 1);
        }
    }else{
        if (value < maximum){
            verticalScrollBar()->setValue(value + 1);
        }
    }
}

CategoryTableWidget::~CategoryTableWidget()
{

}

