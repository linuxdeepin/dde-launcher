#include "categorytablewidget.h"
#include "widgets/util.h"
#include "widgets/themeappicon.h"
#include "app/global.h"
#include "categoryitem.h"
#include "appitem.h"
#include "borderbutton.h"
#include "Logger.h"
#include <QWheelEvent>
#include <QHeaderView>
#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>

static int categoryItemHeight = 80;

CategoryTableWidget::CategoryTableWidget(QWidget *parent) : BaseTableWidget(parent)
{
    setObjectName("CategoryTableWidget");
    initConnect();

    m_scrollAnimation = new QPropertyAnimation;
    m_scrollAnimation->setEasingCurve(QEasingCurve::InCubic);
    m_scrollAnimation->setTargetObject(verticalScrollBar());
    m_scrollAnimation->setPropertyName("value");
    m_scrollAnimation->setDuration(200);
}

void CategoryTableWidget::initConnect(){
    connect(signalManager, SIGNAL(keyDirectionPressed(Qt::Key)),
            this, SLOT(handleDirectionKeyPressed(Qt::Key)));
    connect(signalManager, SIGNAL(categoryInfosChanged(CategoryInfoList)),
            this, SLOT(setCategoryInfoList(CategoryInfoList)));
    connect(signalManager, SIGNAL(itemInfosChanged(QMap<QString,ItemInfo>)),
            this, SLOT(setItemInfosMap(QMap<QString,ItemInfo>)));
    connect(signalManager, SIGNAL(navigationButtonClicked(QString)),
            this, SLOT(scrollToCategory(QString)));
    connect(signalManager, SIGNAL(appOpenedInCategoryMode()),
            this, SLOT(openCheckedApp()));
    connect(signalManager, SIGNAL(showAutoStartLabel(QString)),
            this, SLOT(showAutoStartLabel(QString)));
    connect(signalManager, SIGNAL(hideAutoStartLabel(QString)),
            this, SLOT(hideAutoStartLabel(QString)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(handleScrollBarValueChanged(int)));
    connect(this, SIGNAL(currentAppItemChanged(QString)),
            this, SLOT(handleCurrentAppItemChanged(QString)));
}

void CategoryTableWidget::setGridParameter(int column, int girdWidth, int itemWidth){
    m_column = column;
    m_gridWidth = girdWidth;
    m_itemWidth = itemWidth;
    setColumnCount(m_column);
    for(int i=0; i< m_column; i++){
        setColumnWidth(i, m_gridWidth);
    }

//    int desktopHeight =  qApp->desktop()->screenGeometry().height();
//    int bottomMargin = qApp->desktop()->screenGeometry().height() -
//            qApp->desktop()->availableGeometry().height();
    setFixedWidth(m_column * m_gridWidth);
}

void CategoryTableWidget::setItemInfosMap(const QMap<QString, ItemInfo> &itemInfos){
    m_itemInfosMap.clear();
    m_itemInfosMap = itemInfos;
}

void CategoryTableWidget::setCategoryInfoList(const CategoryInfoList &categoryInfoList){
    qDebug() << "setCategoryInfoList" << categoryInfoList.length();
    m_categoryInfoList = categoryInfoList;

    m_sortedCategoryInfoList.clear();

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
     setRowHeight(row, 60);
     setItem(row, 0, item);

     CategoryItem* categoryItem = new CategoryItem(key);
     categoryItem->setFixedSize(m_gridWidth * m_column, rowHeight(row));
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

            AppItem* appItem = new AppItem(itemInfo.isAutoStart);
            appItem->setAppKey(itemInfo.key);
            appItem->setUrl(itemInfo.url);
            appItem->setAppName(itemInfo.name);
            int size = appItem->getIconSize();
            appItem->setAppIcon(ThemeAppIcon::getIconPixmap(itemInfo.iconKey, size, size));
            appItem->setFixedSize(m_gridWidth, m_gridWidth);
            setCellWidget(_row, column, appItem);
            m_appItems.insert(appKey, appItem);
       }else{
           qDebug() << appKey << m_itemInfosMap.keys();
       }
   }

   if (categoryKey == "others"){
       int desktopHeight =  qApp->desktop()->screenGeometry().height();
       int bottomMargin = qApp->desktop()->screenGeometry().height() - qApp->desktop()->availableGeometry().height();
       int othersCategoryHeight = categoryItemHeight + m_gridWidth * (endRow - startRow);
       if (othersCategoryHeight <  desktopHeight - bottomMargin - 20){
           insertRow(endRow);
           setRowHeight(endRow, height() - othersCategoryHeight + 20);
       }
   }
}

void CategoryTableWidget::addItems(const CategoryInfoList &categoryInfoList){
    clear();
    clearContents();
    m_appItems.clear();
    int rc = rowCount();
    for(int i=0; i< rc; i++){
        removeRow(0);
    }
    verticalScrollBar()->setValue(0);

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

void CategoryTableWidget::removeItem(QString appKey){

}

void CategoryTableWidget::handleCurrentAppItemChanged(QString appKey){
    foreach (CategoryInfo info, m_categoryInfoList) {
        if (info.key != "all" && info.items.contains(appKey)){
            emit signalManager->checkNavigationButtonByKey(info.key);
            break;
        }
    }
}

void CategoryTableWidget::scrollToCategory(QString key){
    int start = verticalScrollBar()->value();
    int end = 0;
    if (m_categoryItems.contains(key)){
        for (int i=0; i< rowCount() ; i++){
            if (m_categoryItems.value(key) == static_cast<CategoryItem*>(cellWidget(i, 0))){
                break;
            } else {
                end += rowHeight(i);
            }
        }
    }

    m_scrollAnimation->setStartValue(start);
    m_scrollAnimation->setEndValue(end);
    m_scrollAnimation->start();
}

void CategoryTableWidget::handleScrollBarValueChanged(int value){
    if (m_scrollAnimation->state() == QPropertyAnimation::Running) return;

    int counter = 0;
    int targetRow = 0;

    // TODO: 50 is not very precise here, the value should be the real height of
    // the others category.
    if (qAbs(value - verticalScrollBar()->maximum()) < 50) {
        emit signalManager->checkNavigationButtonByKey("others");
        return;
    } else {
        for (int i=0; i< rowCount() ; i++){
            counter += rowHeight(i);

            if (value <= counter) {
                targetRow = qMax(0, i - 1);
                break;
            }
        }
    }

    foreach (QString key, m_categoryItems.keys()) {
        if (m_categoryItems.value(key)  == cellWidget(targetRow, 0)){
            emit signalManager->checkNavigationButtonByKey(key);
        }
    }
}

void CategoryTableWidget::openCheckedApp(){
    int hRow = getHighLightRow();
    int hColumn = getHighLightColumn();
    qDebug() << hRow << hColumn << "openCheckedApp";
    if (hRow == -1 && hColumn == -1){
        for (int i=0; i< rowCount(); i++){
            if (cellWidget(i, 0)){
                QString className = QString(cellWidget(i, 0)->metaObject()->className());
                if (className == "AppItem"){
                    AppItem* appItem = static_cast<AppItem*>(cellWidget(i, 0));
                    emit signalManager->appOpened(appItem->getUrl());
                    break;
                }
            }
        }
    }else{
        if (cellWidget(hRow, hColumn)){
            QString className = QString(cellWidget(hRow, hColumn)->metaObject()->className());
            if (className == "AppItem"){
                AppItem* appItem = static_cast<AppItem*>(cellWidget(hRow, hColumn));
                emit signalManager->appOpened(appItem->getUrl());
            }
        }
    }
}

void CategoryTableWidget::showAutoStartLabel(QString appKey){
    if (m_appItems.contains(appKey)){
        reinterpret_cast<AppItem*>(m_appItems.value(appKey))->showAutoStartLabel();
    }
}

void CategoryTableWidget::hideAutoStartLabel(QString appKey){
    if (m_appItems.contains(appKey)){
        reinterpret_cast<AppItem*>(m_appItems.value(appKey))->hideAutoStartLabel();
    }
}

void CategoryTableWidget::wheelEvent(QWheelEvent *event){
    int scrollStep = 40;

    int value = verticalScrollBar()->value();
    int miniimun = verticalScrollBar()->minimum();
    int maximum = verticalScrollBar()->maximum();
    if (event->angleDelta().y() > 0){
        if (value >= miniimun){
            verticalScrollBar()->setValue(value - scrollStep);
        }
    }else{
        if (value < maximum){
            verticalScrollBar()->setValue(value + scrollStep);
        }
    }
}

CategoryTableWidget::~CategoryTableWidget()
{

}

