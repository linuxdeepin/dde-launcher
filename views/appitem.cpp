/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "appitem.h"
#include "widgets/elidelabel.h"
#include "widgets/util.h"
#include "app/global.h"
#include "borderbutton.h"
#include "Logger.h"
#include "views/launcherframe.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>

AppItem::AppItem(bool isAutoStart, QWidget* parent): QFrame(parent),
    m_isAutoStart(isAutoStart)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("AppItem");
    m_delaySaveIconTimer = new QTimer;
    m_delaySaveIconTimer->setSingleShot(true);
    m_delaySaveIconTimer->setInterval(5000);
    initUI();
    initConnect();
    setAppIcon(m_appIcon);
    setAppName(m_appName);
    setMouseTracking(true);
    addTextShadow();
    installEventFilter(this);

}

void AppItem::initUI(){

    int margin = LauncherFrame::AppItemMargin;
    m_borderButton = new BorderButton(this);
    m_borderButton->setFixedSize(LauncherFrame::BorderWidth + margin * 2, LauncherFrame::BorderHeight + margin * 2);

    m_iconLabel = new QLabel(m_borderButton);
    m_iconLabel->setObjectName("Icon");
    m_iconLabel->setScaledContents(true);

    m_autoStartLabel = new QLabel(m_iconLabel);
    m_autoStartLabel->setFixedSize(16, 16);
    m_autoStartLabel->setPixmap(QPixmap(":/images/skin/images/emblem-autostart.png"));

    m_nameLabel = new ElidedLabel(m_borderButton);
    m_nameLabel->setObjectName("Name");
    m_nameLabel->setAlignment(Qt::AlignTop| Qt::AlignHCenter);

    m_iconLabel->setFixedSize(LauncherFrame::IconSize, LauncherFrame::IconSize);
    m_nameLabel->setFixedHeight(LauncherFrame::TextHeight);



    QHBoxLayout* textLayout = new QHBoxLayout;
    textLayout->addStretch();
    textLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    textLayout->addStretch();
    textLayout->setContentsMargins(0, 0, 0, 0);

    m_newInstallIndicatorLabel = new QLabel(m_borderButton);
    m_newInstallIndicatorLabel->setFixedSize(10, 10);
    m_newInstallIndicatorLabel->setPixmap(QPixmap(":/images/skin/img/new_install_indicator.png"));
    m_newInstallIndicatorLabel->hide();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addSpacing(LauncherFrame::AppItemTopSpacing);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(LauncherFrame::AppItemIconNameSpacing);
    mainLayout->addLayout(textLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(margin, margin, margin, margin);
    m_borderButton->setLayout(mainLayout);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_borderButton, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    if (m_isAutoStart){
        showAutoStartLabel();
    }else{
        hideAutoStartLabel();
    }
}

void AppItem::initConnect(){
    connect(m_borderButton, SIGNAL(rightClicked(QPoint)), this, SLOT(showMenu(QPoint)));
    connect(m_borderButton, &BorderButton::clicked, [=](){
        emit signalManager->appOpened(m_appKey);
    });
    connect(signalManager, SIGNAL(rightMouseReleased(QString)), this, SLOT(handleRightMouseReleased(QString)));

    connect(m_borderButton, SIGNAL(graphicsEffectOn()), this, SLOT(disalbelTextShadow()));
    connect(m_borderButton, SIGNAL(graphicsEffectOff()), this, SLOT(addTextShadow()));
    connect(m_delaySaveIconTimer, SIGNAL(timeout()), this, SLOT(delaySaveIconInCache()));
    connect(m_delaySaveIconTimer, SIGNAL(timeout()), m_delaySaveIconTimer, SLOT(deleteLater()));
}

void AppItem::showMenu(QPoint pos){
    qDebug() << m_appKey;
    emit signalManager->contextMenuShowed(m_appKey, pos);
}

QString AppItem::getAppName(){
    return m_appName;
}

void AppItem::setAppName(QString name){
    m_appName = name;
    m_nameLabel->setFullText(name);
}

QPixmap AppItem::getAppIcon(){
    return m_appIcon;
}

void AppItem::setAppIcon(QString icon){
     setAppIcon(QPixmap(icon));
}

void AppItem::setAppIcon(QPixmap icon){
     m_appIcon = icon;
     if (!icon.isNull()){
         m_iconLabel->setPixmap(m_appIcon);
         m_delaySaveIconTimer->start();
     }
}

void AppItem::setUrl(QString url){
    m_url = url;
}

QString AppItem::getAppKey(){
    return m_appKey;
}

QString AppItem::getAppIconKey()
{
    return m_appIconKey;
}

void AppItem::setAppKey(QString key){
    m_appKey = key;
}

void AppItem::setAppIconKey(QString key)
{
    m_appIconKey = key;
}

QString AppItem::getUrl(){
    return m_url;
}

int AppItem::getIconSize(){
    return LauncherFrame::IconSize;
}

BorderButton* AppItem::getBorderButton(){
    return m_borderButton;
}

void AppItem::showAutoStartLabel(){
    m_autoStartLabel->move(0, m_iconLabel->height() - m_autoStartLabel->height());
    m_autoStartLabel->show();
    m_isAutoStart = true;
}

void AppItem::hideAutoStartLabel(){
    m_autoStartLabel->hide();
    m_isAutoStart = false;
}

void AppItem::showNewInstallIndicatorLabel()
{
    m_newInstallIndicatorLabel->show();
//    qDebug() << "m_newInstallIndicatorLabel" << m_newInstallIndicatorLabel->isVisible();
}

void AppItem::hideNewInstallIndicatorLabel()
{

    m_newInstallIndicatorLabel->hide();
//    qDebug() << "m_newInstallIndicatorLabel" << m_newInstallIndicatorLabel->isVisible();
}

void AppItem::setNewInstalled(bool flag)
{
//    qDebug() << m_appKey << flag << "m_isNewInstalled" << m_isNewInstalled;
    m_isNewInstalled = flag;
    if (m_isNewInstalled){
        showNewInstallIndicatorLabel();
    }else{
        hideNewInstallIndicatorLabel();
    }
}

void AppItem::mouseMoveEvent(QMouseEvent *event){
    if (getBorderButton()->geometry().contains(event->pos())){

    }else{
        emit signalManager->mouseReleased();
    }
    QFrame::mouseMoveEvent(event);
}

void AppItem::showEvent(QShowEvent *event)
{
    m_newInstallIndicatorLabel->move(m_nameLabel->pos().x(), m_nameLabel->pos().y() + 5);
    QFrame::showEvent(event);
}



bool AppItem::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::DeferredDelete){
        return true;
    }
    return QFrame::eventFilter(obj, event);
}

void AppItem::mouseReleaseEvent(QMouseEvent *event){
    if (getBorderButton()->geometry().contains(event->pos())){
        emit signalManager->rightMouseReleased(m_url);
    }else{
        emit signalManager->mouseReleased();
    }
    QFrame::mouseReleaseEvent(event);
}

void AppItem::handleRightMouseReleased(QString url){
    if (url == m_url){
        getBorderButton()->setHighlight(true);
    }else{
        getBorderButton()->setHighlight(false);
    }
}

void AppItem::addTextShadow(){
    QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
    textShadow->setBlurRadius(4);
    textShadow->setColor(QColor(0, 0, 0, 128));
    textShadow->setOffset(0, 2);
    m_nameLabel->setGraphicsEffect(textShadow);
}

void AppItem::disalbelTextShadow(){
    if (m_nameLabel->graphicsEffect()){
        m_nameLabel->graphicsEffect()->setEnabled(false);
    }
}

void AppItem::delaySaveIconInCache()
{
    QString cachePath = joinPath(getThumbnailsPath(), QString("%1_%2.png").arg(m_appKey, QString::number(LauncherFrame::IconSize)));
//         qDebug() << m_appIcon.size() << cachePath << QFileInfo(cachePath).exists();
    if (!QFileInfo(cachePath).exists()){
        if (!m_appIcon.isNull()){
            m_appIcon.save(cachePath);
        }
    }
}

AppItem::~AppItem()
{
    qDebug() << this;
}
