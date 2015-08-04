#include "appitem.h"
#include "widgets/elidelabel.h"
#include "widgets/util.h"
#include "Logger.h"


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

AppItem::AppItem(QString icon, QString name, QWidget *parent):
    QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_appIcon = QPixmap(icon);
    m_appName = name;
    setObjectName("AppItem");
    initUI();
    initConnect();

    setAppIcon(m_appIcon);
    setAppName(m_appName);
}

AppItem::AppItem(QPixmap icon, QString name, QWidget *parent):
    QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_appIcon = icon;
    m_appName = name;
    setObjectName("AppItem");
    initUI();
    initConnect();

    setAppIcon(m_appIcon);
    setAppName(m_appName);
}

AppItem::AppItem(QString url, QString icon, QString name, QWidget *parent):
    QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_url = url;
    m_appIcon = QPixmap(icon);
    m_appName = name;
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

    m_borderLabel = new QLabel(this);
    m_borderLabel->setObjectName("BorderLabel");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    m_borderLabel->setLayout(mainLayout);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_borderLabel);
    layout->setSpacing(0);
    layout->setContentsMargins(40, 40, 40, 40);
    setLayout(mainLayout);
}

void AppItem::initConnect(){

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

void AppItem::setAppIcon(QPixmap &icon){
     m_appIcon = icon;
     m_iconLabel->setPixmap(m_appIcon.scaled(m_iconLabel->size()));
}

void AppItem::setUrl(QString url){
    m_url = url;
}

QString AppItem::getUrl(){
    return m_url;
}

void AppItem::resizeEvent(QResizeEvent *event){
    QFrame::resizeEvent(event);
}

void AppItem::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton){
//       emit signalManager->contextMenuShowed(m_url, mapToGlobal(event->pos()));
    }
    QFrame::mousePressEvent(event);
}

void AppItem::enterEvent(QEvent *event){
    m_borderLabel->setStyleSheet("QLabel#BorderLabel{\
                                 background-color: rgba(0, 0, 0, 0.2);\
                                 border: 2px solid rgba(255, 255, 255, 0.15);\
                                 border-radius: 4px;}");
    QFrame::enterEvent(event);
}

void AppItem::leaveEvent(QEvent *event){
    m_borderLabel->setStyleSheet("QLabel#BorderLabel{border: none}");
    QFrame::leaveEvent(event);
}

AppItem::~AppItem()
{
    LOG_INFO() << this << "Desktop Item delete";
}
