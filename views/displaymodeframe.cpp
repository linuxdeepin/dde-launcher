/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "displaymodeframe.h"
#include "widgets/util.h"
#include "basecheckedbutton.h"
#include "baseframe.h"
#include "viewmodebutton.h"
#include "app/global.h"
#include "Logger.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>


DisplayModeFrame::DisplayModeFrame(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
}


void DisplayModeFrame::initUI(){
    setObjectName("DisplayModeFrame");

    m_delayHideTimer = new QTimer(this);
    m_delayHideTimer->setSingleShot(true);
    m_delayHideTimer->setInterval(500);

    m_delayShowTimer = new QTimer(this);
    m_delayShowTimer->setSingleShot(true);
    m_delayShowTimer->setInterval(200);

    m_viewModeButton = new ViewModeButton(this);
    m_viewModeButton->setFixedSize(32, 32);
    m_viewModeButton->move(NavgationBarLeftMargin, 40);

    m_buttonGroup = new QButtonGroup(this);
    m_nameButton = new BaseCheckedButton(tr("By name"), this);
    m_buttonGroup->addButton(m_nameButton, 0);
    m_categoryIconButton = new BaseCheckedButton(tr("By category (icon)"), this);
    m_buttonGroup->addButton(m_categoryIconButton, 1);
    m_categoryNameButton = new BaseCheckedButton(tr("By category (text)"), this);
    m_buttonGroup->addButton(m_categoryNameButton, 2);
    m_installTimeButton = new BaseCheckedButton(tr("By time installed"), this);
    m_buttonGroup->addButton(m_installTimeButton, 3);
    m_useFrequencyButton = new BaseCheckedButton(tr("By frequency"), this);
    m_buttonGroup->addButton(m_useFrequencyButton, 4);


    m_buttonFrame = new BaseFrame(this);
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(m_nameButton);
    buttonLayout->addWidget(m_categoryIconButton);
    buttonLayout->addWidget(m_categoryNameButton);
    buttonLayout->addWidget(m_installTimeButton);
    buttonLayout->addWidget(m_useFrequencyButton);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 20, 0, 0);
    m_buttonFrame->setLayout(buttonLayout);

    m_buttonFrame->move(m_viewModeButton->x(), m_viewModeButton->y() + m_viewModeButton->height());
    setStyleSheet(getQssFromFile(":/qss/skin/qss/buttons.qss"));

    addTextShadow();
}


void DisplayModeFrame::initConnect(){
    connect(signalManager, SIGNAL(viewModeChanged(int)), this, SLOT(checkButton(int)));
    connect(m_viewModeButton, SIGNAL(mouseEnterToggled(bool)), this, SLOT(toggleButtonFrameByViewButton(bool)));
    connect(m_buttonFrame, SIGNAL(mouseEnterToggled(bool)), this, SLOT(toggleButtonFrameBySelf(bool)));
    connect(m_delayHideTimer, SIGNAL(timeout()), this, SLOT(hideButtonFrame()));
    connect(m_delayShowTimer, SIGNAL(timeout()), this, SLOT(showButtonFrame()));
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setViewMode(int)));
}

BaseFrame* DisplayModeFrame::getButtonFrame(){
    return m_buttonFrame;
}

void DisplayModeFrame::toggleButtonFrameByViewButton(bool flag){

    m_viewModeButton->setHoverIconByMode(m_viewMode);
    if (m_viewModeButton->geometry().contains(QCursor::pos())){
        m_delayHideTimer->stop();
    }
    if (flag && m_viewModeButton->geometry().contains(QCursor::pos())){
        if(!m_buttonFrame->isVisible())
            m_delayShowTimer->start();
    }else{
        m_delayShowTimer->stop();
        m_delayHideTimer->start();
    }
}

void DisplayModeFrame::toggleButtonFrameBySelf(bool flag){
    if (!flag){
       m_delayHideTimer->start();
    }else{
        m_delayHideTimer->stop();
    }
}

void DisplayModeFrame::showButtonFrame(){
    int maxWidth = 0;
    foreach (QAbstractButton* button, m_buttonGroup->buttons()) {
        int w = fontMetrics().width(button->text());
        if (w >= maxWidth){
            maxWidth = w;
        }
    }
    setGeometry(0, 0, m_buttonFrame->x() + maxWidth + 10, 220);
    m_buttonFrame->show();
    QPropertyAnimation *animation = new QPropertyAnimation(m_buttonFrame, "geometry");
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->setDuration(100);
    animation->setStartValue(QRect(m_buttonFrame->x(), m_buttonFrame->y(), width() - m_buttonFrame->x(), 72));
    animation->setEndValue(QRect(m_buttonFrame->x(), m_buttonFrame->y(), width() - m_buttonFrame->x(), height() - m_buttonFrame->y()));
    animation->start();
    emit visibleChanged(true);
}

void DisplayModeFrame::hideButtonFrame(){
    int maxWidth = 0;
    foreach (QAbstractButton* button, m_buttonGroup->buttons()) {
        int w = fontMetrics().width(button->text());
        if (w >= maxWidth){
            maxWidth = w;
        }
    }
    setGeometry(0, 0, m_buttonFrame->x() + maxWidth + 10, 72);
    m_buttonFrame->hide();
    m_viewModeButton->setNormalIconByMode(m_viewMode);
    emit visibleChanged(false);
}

int DisplayModeFrame::getCategoryDisplayMode(){
    return m_categoryDisplayMode;
}

void DisplayModeFrame::setCategoryDisplayMode(int mode){
    qDebug() << "setCategoryDisplayMode: !!!" << mode;
    m_categoryDisplayMode = mode;
    emit categoryModeChanged(mode);
    emit signalManager->categoryModeChanged(mode);
}

int DisplayModeFrame::getSortMode(){
    return m_sortMode;
}

void DisplayModeFrame::setSortMode(int mode){
    m_sortMode = mode;
    emit sortModeChanged(mode);
    emit signalManager->sortedModeChanged(mode);
}

void DisplayModeFrame::checkButton(int mode){
    qDebug() << "checkout Buttonx:" << mode;
   m_buttonGroup->button(mode)->click();
}

int DisplayModeFrame::getViewMode() {
    return m_viewMode;
}

void DisplayModeFrame::setViewMode(int mode){
    int lastViewMode = getViewMode();
    if ((mode==1||mode==2) &&(lastViewMode==1||lastViewMode==2)) {
        emit signalManager->categoryFlag(true);
    } else {
        emit signalManager->categoryFlag(false);
    }

    m_viewMode = mode;
    m_viewModeButton->setNormalIconByMode(m_viewMode);
    if (mode == 0){
        setSortMode(mode);
    }else if (mode == 3 || mode == 4){
        setSortMode(mode - 1);
    }else if (mode == 1 || mode ==2){
        setSortMode(1);
        setCategoryDisplayMode(mode - 1);
    }
    hideButtonFrame();
}

void DisplayModeFrame::addTextShadow(){
    QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect(m_buttonFrame);
    textShadow->setBlurRadius(4);
    textShadow->setColor(QColor(0, 0, 0, 128));
    textShadow->setOffset(0, 2);
    m_buttonFrame->setGraphicsEffect(textShadow);
}


DisplayModeFrame::~DisplayModeFrame()
{

}

