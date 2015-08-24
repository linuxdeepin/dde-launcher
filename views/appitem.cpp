#include "appitem.h"
#include "widgets/elidelabel.h"
#include "widgets/util.h"
#include "app/global.h"
#include "borderbutton.h"
#include "Logger.h"
#include <QApplication>
#include "launcherframe.h"

AppItem::AppItem(QWidget *parent) : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_appIcon = QPixmap();
    setObjectName("AppItem");
    initUI();
    initConnect();
    setAppIcon(m_appIcon);
    setAppName(m_appName);
}

void AppItem::initUI(){
    m_iconLabel = new QLabel;
    m_iconLabel->setObjectName("Icon");
    m_iconLabel->setScaledContents(true);
    m_nameLabel = new ElidedLabel;
    m_nameLabel->setObjectName("Name");

    m_iconLabel->setFixedSize(48, 48);

    m_borderButton = new BorderButton(this);
    m_borderButton->setFixedSize(120, 120);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    m_borderButton->setLayout(mainLayout);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_borderButton, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    LauncherFrame::buttonGroup.addButton(m_borderButton);
    setMouseTracking(true);
}

void AppItem::initConnect(){
    connect(m_borderButton, SIGNAL(rightClicked(QPoint)), this, SLOT(showMenu(QPoint)));
    connect(m_borderButton, &BorderButton::clicked, [=](){
        emit signalManager->appOpened(m_appKey);
    });
}

void AppItem::showMenu(QPoint pos){
    qDebug() << m_appKey;
    emit signalManager->contextMenuShowed(m_appKey, pos);
}

QString AppItem::getAppName(){
    return m_appName;
}

void AppItem::setAppName(QString name){
    m_appName = name;
    m_nameLabel->setFullText(name);
}

QPixmap AppItem::getAppIcon(){
    return m_appIcon;
}

void AppItem::setAppIcon(QString icon){
     m_appIcon = QPixmap(icon);
     m_iconLabel->setPixmap(m_appIcon.scaled(m_iconLabel->size()));
}

void AppItem::setAppIcon(QPixmap icon){
     m_appIcon = icon;
     if (!icon.isNull()){
        m_iconLabel->setPixmap(m_appIcon.scaled(m_iconLabel->size()));
     }
}

void AppItem::setUrl(QString url){
    m_url = url;
}

QString AppItem::getAppKey(){
    return m_appKey;
}

void AppItem::setAppKey(QString key){
    m_appKey = key;
}

QString AppItem::getUrl(){
    return m_url;
}

BorderButton* AppItem::getBorderButton(){
    return m_borderButton;
}

void AppItem::mouseMoveEvent(QMouseEvent *event){
    if (getBorderButton()->geometry().contains(event->pos())){

    }else{
        emit signalManager->mouseReleased();
    }
    QFrame::mouseMoveEvent(event);
}

void AppItem::mouseReleaseEvent(QMouseEvent *event){
    if (getBorderButton()->geometry().contains(event->pos())){

    }else{
        emit signalManager->mouseReleased();
    }
    QFrame::mouseReleaseEvent(event);
}

AppItem::~AppItem()
{
    if (LauncherFrame::buttonGroup.buttons().contains(m_borderButton)){
        LauncherFrame::buttonGroup.removeButton(m_borderButton);
    }
}
