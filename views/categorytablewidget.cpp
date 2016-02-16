/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
    qDebug() << this;
    m_scrollAnimation = new QPropertyAnimation(this);
    m_scrollAnimation->setEasingCurve(QEasingCurve::InCubic);
    m_scrollAnimation->setTargetObject(verticalScrollBar());
    m_scrollAnimation->setPropertyName("value");
    m_scrollAnimation->setDuration(200);
}

void CategoryTableWidget::initConnect(){
    connect(signalManager, SIGNAL(keyDirectionPressed(Qt::Key)),
            this, SLOT(handleDirectionKeyPressed(Qt::Key)));

    connect(signalManager, SIGNAL(navigationButtonClicked(qlonglong)),
            this, SLOT(scrollToCategory(qlonglong)));
    connect(signalManager, SIGNAL(appOpenedInCategoryMode()),
            this, SLOT(openCheckedApp()));
    connect(signalManager, SIGNAL(showAutoStartLabel(QString)),
            this, SLOT(showAutoStartLabel(QString)));
    connect(signalManager, SIGNAL(hideAutoStartLabel(QString)),
            this, SLOT(hideAutoStartLabel(QString)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(handleScrollBarValueChanged(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SIGNAL(verticalPositionChanged(int)));
    connect(this, SIGNAL(currentAppItemChanged(QString)),
            this, SLOT(handleCurrentAppItemChanged(QString)));
}

void CategoryTableWidget::show()
{
    BaseTableWidget::show();
}

void CategoryTableWidget::setGridParameter(int column, int girdWidth, int gridHeight, int itemWidth){
    m_column = column;
    m_gridWidth = girdWidth;
    m_gridHeight = gridHeight;
    m_itemWidth = itemWidth;
    setColumnCount(m_column);
    for(int i=0; i< m_column; i++){
        setColumnWidth(i, m_gridWidth);
    }
    setFixedWidth(m_column * m_gridWidth);
}

void CategoryTableWidget::setCategoryInfoList(const CategoryInfoList &categoryInfoList){
    qDebug() << "setCategoryInfoList" << categoryInfoList.length();
    qDebug() << appItemManager->getHideKeys();
    emit signalManager->hideNavigationButtonByKeys(appItemManager->getHideKeys());
    addItems(appItemManager->getSortedCategoryInfoList());
    setItemUnChecked();
}

void CategoryTableWidget::showAppItems()
{
    emit signalManager->hideNavigationButtonByKeys(appItemManager->getHideKeys());
    addItems(appItemManager->getSortedCategoryInfoList());
    setItemUnChecked();
}

void CategoryTableWidget::addCategoryItem(int row, qlonglong key){
     QTableWidgetItem* item = new QTableWidgetItem();
     item->setFlags(Qt::NoItemFlags);
     setSpan(row, 0, 1, m_column);
     setRowHeight(row, 60);
     setItem(row, 0, item);


     CategoryItem* categoryItem = appItemManager->getCategoryItemByKey(key);
     if (categoryItem){
        categoryItem->setParent(this);
        categoryItem->setFixedSize(m_gridWidth * m_column, rowHeight(row));
        setCellWidget(row, 0, categoryItem);
     }
}

void CategoryTableWidget::addItems(int row, qlonglong categoryKey, QStringList appKeys){
    int startRow = row;
    int endRow;

    foreach (QString key, m_keyused) {
        if (appKeys.contains(key)){
            appKeys.removeAll(key);
        }
    }

    if (appKeys.count() % m_column == 0){
        endRow = appKeys.count() / m_column + row;
    }else{
        endRow = appKeys.count() / m_column + row + 1;
    }
    for(int i=startRow; i< endRow; i++){
       insertRow(i);
       setRowHeight(i, m_gridHeight);
    }
    foreach (QString appKey, appKeys) {

       int _row = startRow + appKeys.indexOf(appKey) / m_column;
       int column = appKeys.indexOf(appKey) % m_column;

       AppItem* appItem = appItemManager->getAppItemByKey(appKey);
//       qDebug() << appKey << appItem;
       if (appItem){
           appItem->setParent(this);
           appItem->setFixedSize(m_gridWidth, m_gridHeight);
           setCellWidget(_row, column, appItem);
           appItem->show();
           m_keyused.append(appKey);
       }else{
           qDebug() << appKey;
       }
   }

   if (CategoryID(categoryKey) == CategoryID::Others){
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
    m_keyused.clear();
    clearContents();

    int rc = rowCount();
    for(int i=0; i< rc; i++){
        removeRow(0);
    }
    verticalScrollBar()->setValue(0);

    int row = 0;

    foreach(QString key, CategoryKeys){
        foreach (CategoryInfo info, categoryInfoList) {
            if(info.items.count() > 0 && key == info.name) {
                insertRow(rowCount());
                row = rowCount() - 1;
                addCategoryItem(row, info.id);
//                qDebug() << row << info.name << info.items;
                addItems(rowCount(), info.id,  info.items);
                break;
            }
        }
    }
}

void CategoryTableWidget::removeItem(QString appKey){
    Q_UNUSED(appKey)
}

void CategoryTableWidget::handleCurrentAppItemChanged(QString appKey){
    foreach (CategoryInfo info, appItemManager->getCategoryInfoList()) {
        if (CategoryID(info.id) != CategoryID::All && info.items.contains(appKey)){
            emit signalManager->checkNavigationButtonByKey(info.id);
            break;
        }
    }
}

void CategoryTableWidget::scrollToCategory(qlonglong key){
    int start = verticalScrollBar()->value();
    int end = 0;
    if (appItemManager->getCategoryItems().contains(key)){
        for (int i=0; i< rowCount() ; i++){
            if (appItemManager->getCategoryItems().value(key) == static_cast<CategoryItem*>(cellWidget(i, 0))){
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
        emit signalManager->checkNavigationButtonByKey(qlonglong(CategoryID::Others));
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

    foreach (qlonglong key, appItemManager->getCategoryItems().keys()) {
        if (appItemManager->getCategoryItems().value(key)  == cellWidget(targetRow, 0)){
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

void CategoryTableWidget::showAutoStartLabel(QString appKey){
    if (appItemManager->getAppItems().contains(appKey)){
        reinterpret_cast<AppItem*>(appItemManager->getAppItems().value(appKey))->showAutoStartLabel();
    }
}

void CategoryTableWidget::hideAutoStartLabel(QString appKey){
    if (appItemManager->getAppItems().contains(appKey)){
        reinterpret_cast<AppItem*>(appItemManager->getAppItems().value(appKey))->hideAutoStartLabel();
    }
}

void CategoryTableWidget::handleWheelEvent(QWheelEvent *event){
    wheelEvent(event);
}

void CategoryTableWidget::showEvent(QShowEvent *event)
{
    emit verticalPositionChanged(verticalScrollBar()->value());

    BaseTableWidget::showEvent(event);
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

