#include "borderbutton.h"
#include <QMouseEvent>
#include <QEvent>
#include <QWidget>
#include <QDebug>
#include <QButtonGroup>
#include <QStyle>
#include <QGraphicsOpacityEffect>
#include "app/global.h"
#include "widgets/util.h"
#include "launcherframe.h"
#include "appitem.h"
#include "widgets/elidelabel.h"

BorderButton::BorderButton(QWidget *parent) : QPushButton(parent)
{
//    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("BorderButton");
    setProperty("state", "normal");
    setStyleSheet(getQssFromFile(":/qss/skin/qss/main.qss"));
    addTextShadow();
    connect(signalManager, SIGNAL(highlightChanged(bool)), this, SLOT(setHighlight(bool)));
}

void BorderButton::startDrag(QMouseEvent *event){
    QMimeData* mimeData = new QMimeData;
    QJsonObject obj;
    obj.insert("appKey", dynamic_cast<AppItem*>(parent())->getAppKey());
    obj.insert("appName", dynamic_cast<AppItem*>(parent())->getAppName());
    mimeData->setData("RequestDock", QJsonDocument(obj).toJson());

    QLabel* iconLabel = new QLabel;
    iconLabel->setObjectName("Icon");
    iconLabel->setScaledContents(true);
    iconLabel->setPixmap(dynamic_cast<AppItem*>(parent())->getAppIcon());
    iconLabel->setFixedSize(48, 48);
    iconLabel->setStyleSheet("background-color: transparent");
    QPixmap dragPixmap = iconLabel->grab();
    QDrag* pDrag = new QDrag(this);
    pDrag->setMimeData(mimeData);
    pDrag->setPixmap(dragPixmap);

    pDrag->setHotSpot(event->pos() + QPoint(-36, -10));
//    pDrag->setDragCursor(QCursor(Qt::PointingHandCursor).pixmap(), Qt::MoveAction);

    Qt::DropAction action = pDrag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction);
    if (action == Qt::MoveAction){
        qDebug() << "not support MoveAction";
    }else{
        qDebug() << action;
    }
    iconLabel->deleteLater();
}

void BorderButton::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton){
       emit rightClicked(mapToGlobal(event->pos()));
       return;
    }
    QPushButton::mousePressEvent(event);
}

void BorderButton::mouseMoveEvent(QMouseEvent *event){
    setCuted(true);
    startDrag(event);
    setCuted(false);
    QPushButton::mouseMoveEvent(event);
}

void BorderButton::enterEvent(QEvent *event){
    setHighlight(true);
    QPushButton::enterEvent(event);
}


void BorderButton::leaveEvent(QEvent *event){
    QPushButton::leaveEvent(event);
}

void BorderButton::paintEvent(QPaintEvent *event){
     if (m_isHighlight){
         QPainter painter(this);
         painter.setPen(QPen(QColor(255, 255, 255, 51), 2));
         painter.setBrush(QColor(0, 0 , 0, 76));
         painter.setRenderHint(QPainter::Antialiasing, true);
         painter.drawRoundedRect(QRect(2, 2, 140, 140), 10, 10, Qt::RelativeSize);
     }
     QPushButton::paintEvent(event);
}

void BorderButton::updateStyle(){
    style()->unpolish(this);
    style()->polish(this);
    update();
}

bool BorderButton::isHighlight() const {
    return m_isHighlight;
}

void BorderButton::setHighlight(bool isHightlight){
    m_isHighlight = isHightlight;
    update();
}

void BorderButton::toggleHighlight(){
    setHighlight(!isHighlight());
}

void BorderButton::setCuted(bool isCuted){
    if (m_isCuted == isCuted){
        return;
    }
    if (isCuted){
        setHighlight(false);
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
        effect->setOpacity(0.3);
        setGraphicsEffect(effect);
    }else{
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
        effect->setOpacity(1);
        setGraphicsEffect(effect);
        addTextShadow();
    }
    m_isCuted = isCuted;
}

void BorderButton::addTextShadow(){
    QGraphicsDropShadowEffect *textShadow = new QGraphicsDropShadowEffect;
    textShadow->setBlurRadius(4);
    textShadow->setColor(QColor(0, 0, 0, 128));
    textShadow->setOffset(0, 2);
    setGraphicsEffect(textShadow);
}

BorderButton::~BorderButton()
{

}

