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
#include "launcherframe.h"


BorderButton::BorderButton(QWidget *parent) : QPushButton(parent)
{
//    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("BorderButton");
    setProperty("state", "normal");
    setStyleSheet(getQssFromFile(":/qss/skin/qss/main.qss"));
//    addTextShadow();
    connect(signalManager, SIGNAL(highlightChanged(bool)), this, SLOT(setHighlight(bool)));
}

void BorderButton::startDrag(QMouseEvent *event){
    QMimeData* mimeData = new QMimeData;
    QJsonObject obj;
    obj.insert("appKey", dynamic_cast<AppItem*>(parent())->getAppKey());
    obj.insert("appIcon", dynamic_cast<AppItem*>(parent())->getAppIconKey());
    obj.insert("appName", dynamic_cast<AppItem*>(parent())->getAppName());
    mimeData->setData("RequestDock", QJsonDocument(obj).toJson());

    QLabel* iconLabel = new QLabel;
    iconLabel->setObjectName("Icon");
    iconLabel->setScaledContents(true);
    iconLabel->setPixmap(dynamic_cast<AppItem*>(parent())->getAppIcon());


    iconLabel->setFixedSize(LauncherFrame::IconSize, LauncherFrame::IconSize);
    iconLabel->setStyleSheet("background-color: transparent");
    QPixmap dragPixmap = iconLabel->grab();
    QDrag* pDrag = new QDrag(this);
    pDrag->setMimeData(mimeData);
    pDrag->setPixmap(dragPixmap);

    pDrag->setHotSpot(event->pos() + QPoint(24, 24) - mapFromGlobal(QCursor::pos()));
//    pDrag->setDragCursor(QCursor(Qt::PointingHandCursor).pixmap(), Qt::MoveAction);
    emit signalManager->appItemDragStateChanged(true);
    Qt::DropAction action = pDrag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction);
    if (action == Qt::MoveAction){
        qDebug() << "not support MoveAction";
    }else{
        qDebug() << action;
        emit signalManager->appItemDragStateChanged(false);
    }
    iconLabel->deleteLater();
}

void BorderButton::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton){
       m_isRightPressed = true;
       emit signalManager->rightClickedChanged(true);
       emit rightClicked(mapToGlobal(event->pos()));
       qDebug() << "mousePressEvent";
    }
    QPushButton::mousePressEvent(event);
}

void BorderButton::mouseMoveEvent(QMouseEvent *event){
    emit signalManager->contextMenuHided(dynamic_cast<AppItem*>(parent())->getAppKey());
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
    qDebug() << "leaveEvent";
    if (!m_isRightPressed){
        setHighlight(false);
    }
    m_isRightPressed = false;
    QPushButton::leaveEvent(event);
}

void BorderButton::paintEvent(QPaintEvent *event){
     if (m_isHighlight){
         drawBorder(2, 4, QColor(255, 255, 255, 51), QColor(0, 0 , 0, 76));
     }
     QPushButton::paintEvent(event);
}

void BorderButton::drawBorder(int borderWidth, int radius, QColor borderColor, QColor brushColor){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0, 0, 0, 0), 0));

    QPen pen;
    pen.setColor(borderColor);
    pen.setWidth(borderWidth);
    QPainterPath border;
    border.addRoundedRect(QRectF(borderWidth / 2, borderWidth/2, LauncherFrame::BorderWidth + 2 * LauncherFrame::AppItemMargin - borderWidth, LauncherFrame::BorderHeight + 2 * LauncherFrame::AppItemMargin - borderWidth), radius * 3/4, radius*3/4);
    painter.strokePath(border, pen);

    painter.setBrush(brushColor);
    QRect r(borderWidth, borderWidth, LauncherFrame::BorderWidth + 2 * LauncherFrame::AppItemMargin - borderWidth*2, LauncherFrame::BorderHeight + 2 * LauncherFrame::AppItemMargin - borderWidth*2);
    painter.drawRoundedRect(r, radius, radius);
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
    // mutually exclusive with other BorderButtons.
    if (isHightlight) {
        emit signalManager->highlightChanged(false);
    }

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
        emit graphicsEffectOn();
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
        effect->setOpacity(0.3);
        setGraphicsEffect(effect);
    }else{
        emit graphicsEffectOff();
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
        effect->setOpacity(1);
        setGraphicsEffect(effect);
        graphicsEffect()->setEnabled(false);
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

