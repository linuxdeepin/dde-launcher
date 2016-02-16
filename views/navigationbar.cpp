/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "navigationbar.h"
#include "navigationbuttonframe.h"
#include "widgets/util.h"
#include "app/global.h"
#include <QStackedLayout>
#include <QApplication>
#include <QDesktopWidget>

NavigationBar::NavigationBar(QWidget *parent) : QFrame(parent)
{
    setObjectName("NavigationBar");
}


void NavigationBar::initUI(int width){
    m_iconFrame = new NavigationButtonFrame(0, this);
    m_textFrame = new NavigationButtonFrame(1, this);
    m_layout = new QStackedLayout(this);
    m_layout->addWidget(m_iconFrame);
    m_layout->addWidget(m_textFrame);
    m_layout->setCurrentIndex(0);
    setLayout(m_layout);
    setFixedSize(width, qApp->desktop()->screenGeometry().height());
    setStyleSheet(getQssFromFile(":/qss/skin/qss/buttons.qss"));
    initConnect();
}

void NavigationBar::initConnect(){
    connect(m_iconFrame, SIGNAL(currentIndexChanged(int)), m_textFrame, SLOT(setCurrentIndex(int)));
    connect(m_textFrame, SIGNAL(currentIndexChanged(int)), m_iconFrame, SLOT(setCurrentIndex(int)));
    connect(signalManager, SIGNAL(hightlightButtonByIndex(int)), m_textFrame, SLOT(setCurrentIndex(int)));
    connect(signalManager, SIGNAL(hightlightButtonByIndex(int)), m_iconFrame, SLOT(setCurrentIndex(int)));
}

NavigationButtonFrame* NavigationBar::getIconFrame(){
    return m_iconFrame;
}

NavigationButtonFrame* NavigationBar::getTextFrame(){
    return m_textFrame;
}

void NavigationBar::setCurrentIndex(int index){
    m_layout->setCurrentIndex(index);
}

NavigationBar::~NavigationBar()
{

}

