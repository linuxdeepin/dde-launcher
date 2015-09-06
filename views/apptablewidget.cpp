#include "apptablewidget.h"
#include "widgets/util.h"
#include "widgets/themeappicon.h"
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
    initConnect();
}

void AppTableWidget::initConnect(){
    connect(signalManager, SIGNAL(keyDirectionPressed(Qt::Key)),
            this, SLOT(handleDirectionKeyPressed(Qt::Key)));
    connect(signalManager, SIGNAL(appNameItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(setAppNameItemInfoList(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(installTimeItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(setInstallTimeItemInfoList(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(useFrequencyItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(setuseFrequencyItemInfoList(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(searchItemInfoListChanged(QList<ItemInfo>)),
            this, SLOT(showSearchResult(QList<ItemInfo>)));
    connect(signalManager, SIGNAL(itemInfosChanged(QMap<QString,ItemInfo>)), this, SLOT(setItemInfosMap(QMap<QString,ItemInfo>)));

    connect(signalManager, SIGNAL(appOpenedInAppMode()), this, SLOT(openCheckedApp()));

    connect(signalManager, SIGNAL(showAutoStartLabel(QString)),
            this, SLOT(showAutoStartLabel(QString)));
    connect(signalManager, SIGNAL(hideAutoStartLabel(QString)),
            this, SLOT(hideAutoStartLabel(QString)));
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

    AppItem* appItem = new AppItem(itemInfo.isAutoStart);
    appItem->setAppKey(itemInfo.key);
    appItem->setUrl(itemInfo.url);
    appItem->setAppName(itemInfo.name);
    appItem->setAppIcon(ThemeAppIcon::getIconPixmap(itemInfo.iconKey));
    appItem->setFixedSize(m_gridWidth, m_gridWidth);
    setCellWidget(row, column, appItem);

    m_appItems.insert(itemInfo.key, appItem);
}


void AppTableWidget::addItems(const QList<ItemInfo> &itemInfos){
    clear();
    clearContents();
    m_appItems.clear();
    int rc = rowCount();
    for(int i=0; i< rc; i++){
        removeRow(0);
    }
    clearHighlight();

    verticalScrollBar()->setValue(0);
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
    m_itemInfosMap.clear();
    m_itemInfosMap = itemInfos;
}

void AppTableWidget::showBySortedMode(int mode){
    if (mode == 0){
        showbyName();
    }else if (mode == 2){
        showByInstalledTime();
    }else if (mode == 3){
        showByFrequency();
    }
    clearHighlight();
    m_currentMode = mode;
}

void AppTableWidget::setAppNameItemInfoList(const QList<ItemInfo> &infoList){
    m_appNameItemInfoList.clear();
    m_appNameItemInfoList = infoList;
}

void AppTableWidget::setInstallTimeItemInfoList(const QList<ItemInfo> &infoList){
    m_InstalltimeItemInfoList.clear();
    m_InstalltimeItemInfoList= infoList;
}

void AppTableWidget::setuseFrequencyItemInfoList(const QList<ItemInfo> &infoList){
    m_useFrequencyItemInfoList.clear();
    m_useFrequencyItemInfoList = infoList;
}

void AppTableWidget::showbyName(){
    addItems(m_appNameItemInfoList);
}

void AppTableWidget::showByInstalledTime(){
    addItems(m_InstalltimeItemInfoList);
}

void AppTableWidget::showByFrequency(){
    addItems(m_useFrequencyItemInfoList);
}

void AppTableWidget::showSearchResult(const QList<ItemInfo> &infoList){
    addItems(infoList);
}

void AppTableWidget::showNormalMode(){
    showBySortedMode(m_currentMode);
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

void AppTableWidget::openCheckedApp(){
    int hRow = getHighLightRow();
    int hColumn = getHighLightColumn();
    if (hRow == -1 && hColumn == -1){
        for (int i=0; i< rowCount(); i++){
            if (cellWidget(0, 0)){
                QString className = QString(cellWidget(0, 0)->metaObject()->className());
                if (className == "AppItem"){
                    AppItem* appItem = static_cast<AppItem*>(cellWidget(0, 0));
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

void AppTableWidget::showAutoStartLabel(QString appKey){
    if (m_appItems.contains(appKey)){
        reinterpret_cast<AppItem*>(m_appItems.value(appKey))->showAutoStartLabel();
    }
}

void AppTableWidget::hideAutoStartLabel(QString appKey){
    if (m_appItems.contains(appKey)){
        reinterpret_cast<AppItem*>(m_appItems.value(appKey))->hideAutoStartLabel();
    }
}

AppTableWidget::~AppTableWidget()
{

}

