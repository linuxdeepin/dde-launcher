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
    connect(signalManager, SIGNAL(hideNavigationButtonByKeys(QList<qlonglong>)), this, SLOT(hideButtons(QList<qlonglong>)));
    connect(signalManager, SIGNAL(checkNavigationButtonByKey(qlonglong)), this, SLOT(checkButtonByKey(qlonglong)));
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleButtonClicked(int)));
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentIndex(int)));
    connect(signalManager, SIGNAL(firstButtonChecked()), this, SLOT(checkFirstButton()));
}

void NavigationButtonFrame::initByMode(int mode){
    setObjectName("NavigationButtonFrame");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();

    QFrame* buttonFrame = new QFrame;
    buttonFrame->setAttribute(Qt::WA_NoMousePropagation);
    QVBoxLayout* buttonLayout = new QVBoxLayout;
     foreach (QString key, CategoryKeys) {
        if (mode == 0){
            BaseCheckedButton* button = new BaseCheckedButton(this);
            button->setFixedSize(32, 32);
            button->setObjectName(key.toLower());
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategoryKeys.indexOf(key));
            button->hide();
        }else{
            qDebug() << "navigation button frame" << key;
            QString name = getCategoryNames(key);
            BaseCheckedButton* button = new BaseCheckedButton(name, this);
            button->setObjectName("CategoryTextButton");
            button->setFixedSize(160 - NavgationBarLeftMargin, 24);
            buttonLayout->addWidget(button);
            m_buttonGroup->addButton(button, CategoryKeys.indexOf(key));
            button->hide();
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

void NavigationButtonFrame::hideButtons(const QList<qlonglong> &keys){
    for (int key = 0, len = CategoryKeys.size(); key < len; ++key) {
        if (!keys.contains(key)){
            int index = key;
            m_buttonGroup->button(index)->show();
        }else{
            int index = key;
            m_buttonGroup->button(index)->hide();
        }
    }
}


void NavigationButtonFrame::handleButtonClicked(int id){
    emit signalManager->navigationButtonClicked(id);
    emit currentIndexChanged(id);
}

void NavigationButtonFrame::checkButtonByKey(qlonglong key){
    int index = key;
    emit currentIndexChanged(index);
//    qDebug() << "currentIndexChanged" << index;
}

void NavigationButtonFrame::setCurrentIndex(int currentIndex){
//    qDebug() << this << m_currentIndex << currentIndex;
    if (m_currentIndex == currentIndex)
        return;
    m_currentIndex = currentIndex;
    m_buttonGroup->button(currentIndex)->setChecked(true);
}

void NavigationButtonFrame::checkFirstButton(){
    for(int i=0; i<m_buttonGroup->buttons().length(); i++){
        if (m_buttonGroup->button(i)->isVisible()){
            m_buttonGroup->button(i)->click();
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

