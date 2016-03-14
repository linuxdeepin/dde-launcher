/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "navigationbuttonframe.h"
#include "widget/basecheckedbutton.h"
//#include "Logger.h"
#include "global_util/util.h"
#include "model/appslistmodel.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

NavigationButtonFrame::NavigationButtonFrame(QWidget *parent) : QFrame(parent)
{
    setFixedWidth(140);
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    int m_mode = 0;
    initByMode(m_mode);
    initConnect();
}

void NavigationButtonFrame::initConnect(){
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleButtonClicked(int)));
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentIndex(int)));
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
//            QString name = getCategoryNames(key);
            BaseCheckedButton* button = new BaseCheckedButton(key, this);
            button->setObjectName("CategoryTextButton");
            //NavgationBarLeftMargin = 50;
            button->setFixedSize(160 - 50, 24);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategoryKeys.indexOf(key));
        }
    }
    buttonLayout->setSpacing(24);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonFrame->setLayout(buttonLayout);

    layout->addWidget(buttonFrame);
    m_buttonGroup->buttons().at(0)->click();
    //NavgationBarLeftMargin = 50;
    layout->setContentsMargins(50, 0, 0, 0);
    layout->addStretch();
    setLayout(layout);

    if (mode == 1){
        addTextShadow();
    }

     setStyleSheet(getQssFromFile(":/skin/qss/buttons.qss"));
}

void NavigationButtonFrame::hideButtons(const QList<qlonglong> &keys){
    for (int key = 0, len = CategoryKeys.size(); key < len; ++key) {
        if (!keys.contains(key)){
            int index = key;
            m_buttonGroup->button(index)->show();
        }else{
            int index = key;
            m_buttonGroup->button(index)->hide();
            qDebug() << "hideButtons^^^^^^^^^^^^^^^***:" << index;
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

