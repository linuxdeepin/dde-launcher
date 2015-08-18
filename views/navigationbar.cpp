#include "navigationbar.h"
#include "navigationbuttonframe.h"
#include "widgets/util.h"
#include <QStackedLayout>
#include <QApplication>
#include <QDesktopWidget>

NavigationBar::NavigationBar(QWidget *parent) : QFrame(parent)
{

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
}

void NavigationBar::initConnect(){

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

