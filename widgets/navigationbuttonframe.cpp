/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "navigationbuttonframe.h"
#include "basecheckedbutton.h"
//#include "Logger.h"
#include "global_util/global.h"
#include "model/appslistmodel.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

NavigationButtonFrame::NavigationButtonFrame(int mode, QWidget *parent) : QFrame(parent)
{
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    initByMode(mode);
    initConnect();
    updateUI();
}

void NavigationButtonFrame::initConnect(){
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleButtonClicked(int)));
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), signalManager, SIGNAL(scrollToCategory(int)));
    connect(signalManager, SIGNAL(scrollToCategory(int)), this, SLOT(testing(int)));
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentIndex(int)));

}

void NavigationButtonFrame::testing(int index) {
    qDebug() << "111111111111111111 index" << index;
}

void NavigationButtonFrame::initByMode(int mode){
    setObjectName("NavigationButtonFrame");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    QFrame* buttonFrame = new QFrame;
    buttonFrame->setAttribute(Qt::WA_NoMousePropagation);
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    qDebug() << "CategoryKeys***:" << CategoryKeys;
     foreach (QString key, CategoryKeys) {
        if (mode == 0){
            BaseCheckedButton* button = new BaseCheckedButton(this);
            button->setFixedSize(32, 32);
            qDebug() << "key.toLower" << key.toLower();
            button->setObjectName(key.toLower());
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategoryKeys.indexOf(key));
        } else {
            qDebug() << "navigation button frame" << key;
            QString name = getCategoryNames(key);
            BaseCheckedButton* button = new BaseCheckedButton(name, this);
            button->setObjectName("CategoryTextButton");
            button->setFixedSize(160 - NavgationBarLeftMargin, 24);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategoryKeys.indexOf(key));
        }
    }
    buttonLayout->setSpacing(24);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonFrame->setLayout(buttonLayout);

    layout->addWidget(buttonFrame);
    m_buttonGroup->buttons().at(0)->click();
    layout->setContentsMargins(NavgationBarLeftMargin, 0, 0, 0);
    layout->addStretch();
    setLayout(layout);

    if (mode == 1){
        addTextShadow();
    }
}

void NavigationButtonFrame::updateUI() {
    hideButtons();
}

void NavigationButtonFrame::hideButtons(){
    QList<int> appNumList = appsManager->getCategoryAppNumsList();
    for(int i(0); i< appNumList.size();i++) {
        if (appNumList[i]==0) {
            qDebug() << "i is the button need to be hided:" << i;
            m_buttonGroup->button(i)->hide();

        }
    }
}


void NavigationButtonFrame::handleButtonClicked(int id){
    emit currentIndexChanged(id);
    qDebug() << "handleButtonClicked*********" << id;
}
/*
    check the currentButton in navigationBar, when scroll the categoryTable
*/

void NavigationButtonFrame::checkButtonByKey(qlonglong key){
    int index = key;
    emit currentIndexChanged(index);
    qDebug() << "scroll to currentIndex***** :" << index;
}

void NavigationButtonFrame::setCurrentIndex(int currentIndex){
    qDebug() << "NavigationButtonFrame$$$";
    qDebug() << this << m_currentIndex << currentIndex;
    if (m_currentIndex == currentIndex)
        return;
    m_currentIndex = currentIndex;
    m_buttonGroup->button(currentIndex)->setChecked(true);
    qDebug() << "###########m_buttonGroup:" << currentIndex;
}

void NavigationButtonFrame::checkFirstButton(){
    qDebug() << "checkFirstButton***";
    for(int i=0; i<m_buttonGroup->buttons().length(); i++){
        if (m_buttonGroup->button(i)->isVisible()){
            m_buttonGroup->button(i)->click();
            emit currentIndexChanged(i);
            break;
        }
    }
}

void NavigationButtonFrame::addTextShadow(){
    QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
    textShadow->setBlurRadius(4);
    textShadow->setColor(QColor(0, 0, 0, 128));
    textShadow->setOffset(0, 2);
    setGraphicsEffect(textShadow);
}

NavigationButtonFrame::~NavigationButtonFrame()
{

}

