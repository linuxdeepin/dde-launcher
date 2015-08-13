#include "navigationbuttonframe.h"
#include "basecheckedbutton.h"
#include "Logger.h"
#include "app/global.h"
#include <QVBoxLayout>
#include <QButtonGroup>


NavigationButtonFrame::NavigationButtonFrame(int mode, QWidget *parent) : QFrame(parent)
{
    m_categroyKeys << "internet" << "multimedia" << "games";
    m_categroyKeys << "graphics" << "productivity" << "industry";
    m_categroyKeys << "education" << "development" << "system";
    m_categroyKeys << "utilities" << "others";

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    initByMode(mode);
    initConnect();
}

void NavigationButtonFrame::initConnect(){
    connect(signalManager, SIGNAL(hideNavigationButtonByKeys(QStringList)), this, SLOT(hideButtons(QStringList)));
    connect(signalManager, SIGNAL(checkNavigationButtonByKey(QString)), this, SLOT(checkButtonByKey(QString)));
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleButtonClicked(int)));
}

void NavigationButtonFrame::initByMode(int mode){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    foreach (QString key, m_categroyKeys) {
        if (mode == 0){
            BaseCheckedButton* button = new BaseCheckedButton(this);
            button->setFixedSize(32, 32);
//            LOG_INFO() << button << key;
            button->setObjectName(key);
            layout->addWidget(button);
            m_buttonGroup->addButton(button, m_categroyKeys.indexOf(key));
        }else{
            BaseCheckedButton* button = new BaseCheckedButton(key, this);
            button->setObjectName("CategoryTextButton");
            button->setFixedSize(160 - leftMargin, 24);
            layout->addWidget(button);
            m_buttonGroup->addButton(button, m_categroyKeys.indexOf(key));
        }
    }
    m_buttonGroup->buttons().at(0)->click();
    layout->setSpacing(20);
    layout->setContentsMargins(leftMargin, 0, 0, 0);
    layout->addStretch();
    setLayout(layout);
}

void NavigationButtonFrame::hideButtons(const QStringList &keys){
    foreach (QString key, keys) {
        int index = m_categroyKeys.indexOf(key);
        m_buttonGroup->button(index)->hide();
    }
}


void NavigationButtonFrame::handleButtonClicked(int id){
    emit signalManager->navigationButtonClicked(m_categroyKeys.at(id));
}

void NavigationButtonFrame::checkButtonByKey(QString key){
    int index = m_categroyKeys.indexOf(key);
    m_buttonGroup->button(index)->setChecked(true);
}

NavigationButtonFrame::~NavigationButtonFrame()
{

}

