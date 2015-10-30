#include "appitem.h"
#include "widgets/elidelabel.h"
#include "widgets/util.h"
#include "app/global.h"
#include "borderbutton.h"
#include "Logger.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>

AppItem::AppItem(bool isAutoStart, QWidget* parent): QFrame(parent),
    m_isAutoStart(isAutoStart)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("AppItem");
    initUI();
    initConnect();
    setAppIcon(m_appIcon);
    setAppName(m_appName);
    setMouseTracking(true);
}

void AppItem::initUI(){
    m_iconLabel = new QLabel;
    m_iconLabel->setObjectName("Icon");
    m_iconLabel->setScaledContents(true);

    m_autoStartLabel = new QLabel(m_iconLabel);
    m_autoStartLabel->setFixedSize(16, 16);
    m_autoStartLabel->setPixmap(QPixmap(":/images/skin/images/emblem-autostart.png"));

    m_nameLabel = new ElidedLabel;
    m_nameLabel->setObjectName("Name");
    m_nameLabel->setAlignment(Qt::AlignTop| Qt::AlignHCenter);

    m_iconLabel->setFixedSize(64, 64);
    m_nameLabel->setFixedSize(100, 34);

    m_borderButton = new BorderButton(this);
    m_borderButton->setFixedSize(144, 144);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addSpacing(14);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    m_borderButton->setLayout(mainLayout);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_borderButton, Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    if (m_isAutoStart){
        showAutoStartLabel();
    }else{
        hideAutoStartLabel();
    }
}

void AppItem::initConnect(){
    connect(m_borderButton, SIGNAL(rightClicked(QPoint)), this, SLOT(showMenu(QPoint)));
    connect(m_borderButton, &BorderButton::clicked, [=](){
        emit signalManager->appOpened(m_url);
    });
    connect(signalManager, SIGNAL(rightMouseReleased(QString)), this, SLOT(handleRightMouseReleased(QString)));
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
     setAppIcon(QPixmap(icon));
}

void AppItem::setAppIcon(QPixmap icon){
     m_appIcon = icon;
     if (!icon.isNull()){
//        m_iconLabel->setPixmap(m_appIcon.scaled(m_iconLabel->size()));
         m_iconLabel->setPixmap(m_appIcon);
         QString cachePath = joinPath(getThumbnailsPath(), QString("%1.png").arg(m_appKey));
         qDebug() << m_appIcon.size() << cachePath << QFileInfo(cachePath).exists();
         if (!QFileInfo(cachePath).exists())
            m_appIcon.save(cachePath);
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

void AppItem::showAutoStartLabel(){
    m_autoStartLabel->move(0, m_iconLabel->height() - m_autoStartLabel->height());
    m_autoStartLabel->show();
    m_isAutoStart = true;
}

void AppItem::hideAutoStartLabel(){
    m_autoStartLabel->hide();
    m_isAutoStart = false;
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
        emit signalManager->rightMouseReleased(m_url);
    }else{
        emit signalManager->mouseReleased();
    }
    QFrame::mouseReleaseEvent(event);
}

void AppItem::handleRightMouseReleased(QString url){
    if (url == m_url){
        getBorderButton()->setHighlight(true);
    }else{
        getBorderButton()->setHighlight(false);
    }
}

AppItem::~AppItem()
{
}
