#include "navigationbuttonframe.h"
#include "basecheckedbutton.h"
#include "Logger.h"
#include "app/global.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>

NavigationButtonFrame::NavigationButtonFrame(int mode, QWidget *parent) : QFrame(parent)
{
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    initByMode(mode);
    initConnect();
}

void NavigationButtonFrame::initConnect(){
    connect(signalManager, SIGNAL(hideNavigationButtonByKeys(QStringList)), this, SLOT(hideButtons(QStringList)));
    connect(signalManager, SIGNAL(checkNavigationButtonByKey(QString)), this, SLOT(checkButtonByKey(QString)));
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
    foreach (QString key, CategroyKeys) {
        if (mode == 0){
            BaseCheckedButton* button = new BaseCheckedButton(this);
            button->setFixedSize(32, 32);
            button->setObjectName(key);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategroyKeys.indexOf(key));
            button->hide();
        }else{
            int index = CategroyKeys.indexOf(key);
            QString name = CategoryNames.at(index);
            BaseCheckedButton* button = new BaseCheckedButton(name, this);
            button->setObjectName("CategoryTextButton");
            button->setFixedSize(160 - NavgationBarLeftMargin, 24);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategroyKeys.indexOf(key));
            button->hide();
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
    foreach (QString key, CategroyKeys) {
        if (!keys.contains(key)){
            int index = CategroyKeys.indexOf(key);
            m_buttonGroup->button(index)->show();
        }
    }
}


void NavigationButtonFrame::handleButtonClicked(int id){
    emit signalManager->navigationButtonClicked(CategroyKeys.at(id));
    emit currentIndexChanged(id);
}

void NavigationButtonFrame::checkButtonByKey(QString key){
    int index = CategroyKeys.indexOf(key);
    emit currentIndexChanged(index);
    qDebug() << "currentIndexChanged" << index;
}

void NavigationButtonFrame::setCurrentIndex(int currentIndex){
    qDebug() << this << m_currentIndex << currentIndex;
    if (m_currentIndex == currentIndex)
        return;
    m_currentIndex = currentIndex;
    m_buttonGroup->button(currentIndex)->setChecked(true);
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

