#include "navigationlistwidget.h"
#include "global_util/constants.h"
#include "global_util/util.h"

#include <QVBoxLayout>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QLabel>

NavigationListWidget::NavigationListWidget(QWidget *parent) :
    QWidget(parent)
{
    setObjectName("NavigationBar");
    initUI(140);
    initConnect();
}


void NavigationListWidget::initUI(int width){
    m_iconFrame = new NavigationButtonFrame(0, this);
    m_textFrame = new NavigationButtonFrame(1, this);
    m_layout = new QStackedLayout(this);
    m_layout->addWidget(m_iconFrame);
    m_layout->addWidget(m_textFrame);
    m_layout->setCurrentIndex(0);
    setLayout(m_layout);
    setFixedSize(width, qApp->desktop()->screenGeometry().height());
    setStyleSheet(getQssFromFile(":/skin/qss/buttons.qss"));
    initConnect();
}

void NavigationListWidget::initConnect(){
//    connect(m_iconFrame, SIGNAL(currentIndexChanged(int)), m_textFrame, SLOT(setCurrentIndex(int)));


    //    connect(signalManager, SIGNAL(hightlightButtonByIndex(int)), m_textFrame, SLOT(setCurrentIndex(int)));
//    connect(signalManager, SIGNAL(hightlightButtonByIndex(int)), m_iconFrame, SLOT(setCurrentIndex(int)));

//    connect(m_iconFrame, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentIndex(int)));
//    connect(m_textFrame, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentIndex(int)));
}

NavigationButtonFrame* NavigationListWidget::getIconFrame(){
    return m_iconFrame;
}

NavigationButtonFrame* NavigationListWidget::getTextFrame(){
    return m_textFrame;
}

void NavigationListWidget::setCurrentIndex(int index){
    qDebug() << "navigationButtonFrme:************" << index;
//    m_layout->setCurrentIndex(index);
//    m_iconFrame->setCurrentIndex(index);
//    emit currentIndexChanged(index);
}

NavigationListWidget::~NavigationListWidget()
{

}
