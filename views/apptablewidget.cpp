#include "apptablewidget.h"
#include "widgets/util.h"
#include "widgets/themeappicon.h"
#include "app/global.h"
#include "dbusinterface/dbustype.h"
#include "Logger.h"
#include "appitem.h"
#include "borderbutton.h"
#include "dbusinterface/dbustype.h"
#include "fwd.h"
#include "launcherframe.h"

#include <QTableWidgetItem>
#include <QWheelEvent>
#include <QShowEvent>
#include <QScrollBar>


AppTableWidget::AppTableWidget(QWidget *parent) : BaseTableWidget(parent)
{
    m_currentMode = 0;
    setObjectName("AppTableWidget");
    initConnect();
}

void AppTableWidget::initConnect(){
    connect(signalManager, SIGNAL(keyDirectionPressed(Qt::Key)),
            this, SLOT(handleDirectionKeyPressed(Qt::Key)));

    connect(signalManager, SIGNAL(appOpenedInAppMode()), this, SLOT(openCheckedApp()));

    connect(signalManager, SIGNAL(showAutoStartLabel(QString)),
            this, SLOT(showAutoStartLabel(QString)));
    connect(signalManager, SIGNAL(hideAutoStartLabel(QString)),
            this, SLOT(hideAutoStartLabel(QString)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SIGNAL(contentScrolled(int)));
}

void AppTableWidget::setGridParameter(int column, int girdWidth, int gridHeight, int itemWidth){
    m_column = column;
    m_gridWidth = girdWidth;
    m_gridHeight = gridHeight;
    m_itemWidth = itemWidth;
    setColumnCount(m_column);
    for(int i=0; i< m_column; i++){
        setColumnWidth(i, m_gridWidth);
    }
}

int AppTableWidget::getCurrentMode(){
    return m_currentMode;
}

void AppTableWidget::addItem(ItemInfo itemInfo, int index){
    int row = index / m_column;
    int column = index % m_column;
    AppItem* appItem = appItemManager->getAppItemByKey(itemInfo.key);
    if (appItem){
        appItem->setParent(this);
        appItem->setFixedSize(m_gridWidth, m_gridHeight);
        setCellWidget(row, column, appItem);
        appItem->show();
    }else{
        qDebug() << itemInfo.key;
    }
    emit signalManager->highlightChanged(false);
}


void AppTableWidget::addItems(const QList<ItemInfo> &itemInfos){
    clearContents();
    int rc = rowCount();
    for(int i=0; i< rc; i++){
        removeRow(0);
    }

    clearHighlight();

    verticalScrollBar()->setValue(0);
    setRowCount(itemInfos.length() / m_column + 1);
    for(int i=0; i< rowCount(); i++){
        setRowHeight(i, m_gridHeight);
    }
    setItemUnChecked();

    for (int i=0; i< itemInfos.length(); i++) {
        addItem(itemInfos.at(i), i);
    }
}

void AppTableWidget::showBySortedMode(int mode){
    qDebug() << mode;
    if (mode == 0){
        showbyName();
    }else if (mode == 2){
        showByInstalledTime();
    }else if (mode == 3){
        showByFrequency();
    }
    clearHighlight();
    if (m_currentMode != mode){
        m_currentMode = mode;
    }
}

void AppTableWidget::setAppNameItemInfoList(const QList<ItemInfo> &infoList){
    Q_UNUSED(infoList)
    qDebug() << "setAppNameItemInfoList";
    showBySortedMode(m_currentMode);
}

void AppTableWidget::setInstallTimeItemInfoList(const QList<ItemInfo> &infoList){
    Q_UNUSED(infoList)
    qDebug() << "setInstallTimeItemInfoList";
    showBySortedMode(m_currentMode);
}

void AppTableWidget::setuseFrequencyItemInfoList(const QList<ItemInfo> &infoList){
    Q_UNUSED(infoList)
    qDebug() << "setuseFrequencyItemInfoList";
    showBySortedMode(m_currentMode);
}

void AppTableWidget::showbyName(){
    addItems(appItemManager->getAppNameItemInfos());
}

void AppTableWidget::showByInstalledTime(){
    addItems(appItemManager->getInstallTimeItemInfos());
}

void AppTableWidget::showByFrequency(){
    addItems(appItemManager->getUseFrequencyItemInfos());
}

void AppTableWidget::showSearchResult(const QList<ItemInfo> &infoList){
    addItems(infoList);
}

void AppTableWidget::showNormalMode(){
    showBySortedMode(m_currentMode);
}

void AppTableWidget::handleWheelEvent(QWheelEvent *event){
    wheelEvent(event);
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

void AppTableWidget::showEvent(QShowEvent * event)
{
    emit showed();
    emit contentScrolled(verticalScrollBar()->value());

    BaseTableWidget::showEvent(event);
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
                    emit signalManager->appOpened(appItem->getAppKey());
                    break;
                }
            }
        }
    }else{
        if (cellWidget(hRow, hColumn)){
            QString className = QString(cellWidget(hRow, hColumn)->metaObject()->className());
            if (className == "AppItem"){
                AppItem* appItem = static_cast<AppItem*>(cellWidget(hRow, hColumn));
                emit signalManager->appOpened(appItem->getAppKey());
            }
        }
    }
}

void AppTableWidget::showAutoStartLabel(QString appKey){
    if (appItemManager->getAppItems().contains(appKey)){
        reinterpret_cast<AppItem*>(appItemManager->getAppItems().value(appKey))->showAutoStartLabel();
    }
}

void AppTableWidget::hideAutoStartLabel(QString appKey){
    if (appItemManager->getAppItems().contains(appKey)){
        reinterpret_cast<AppItem*>(appItemManager->getAppItems().value(appKey))->hideAutoStartLabel();
    }
}

AppTableWidget::~AppTableWidget()
{

}

