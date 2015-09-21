#include "navigationbuttonframe.h"
#include "basecheckedbutton.h"
#include "Logger.h"
#include "app/global.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>


NavigationButtonFrame::NavigationButtonFrame(int mode, QWidget *parent) : QFrame(parent)
{
    m_categroyKeys << tr("internet") << tr("multimedia") << tr("games");
    m_categroyKeys << tr("graphics") << tr("productivity") << tr("industry");
    m_categroyKeys << tr("education") << tr("development") << tr("system");
    m_categroyKeys << tr("utilities") << tr("others");

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
    setObjectName("NavigationButtonFrame");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();

    QFrame* buttonFrame = new QFrame;
    buttonFrame->setAttribute(Qt::WA_NoMousePropagation);
    QVBoxLayout* buttonLayout = new QVBoxLayout;
    foreach (QString key, m_categroyKeys) {
        if (mode == 0){
            BaseCheckedButton* button = new BaseCheckedButton(this);
            button->setFixedSize(32, 32);
            button->setObjectName(key);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, m_categroyKeys.indexOf(key));
        }else{
            BaseCheckedButton* button = new BaseCheckedButton(key, this);
            button->setObjectName("CategoryTextButton");
            button->setFixedSize(160 - NavgationBarLeftMargin, 24);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, m_categroyKeys.indexOf(key));
        }
    }
    buttonLayout->setSpacing(20);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonFrame->setLayout(buttonLayout);

    layout->addWidget(buttonFrame);
    m_buttonGroup->buttons().at(0)->click();
    layout->setContentsMargins(NavgationBarLeftMargin, 0, 0, 0);
    layout->addStretch();
    setLayout(layout);

    addTextShadow();
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

