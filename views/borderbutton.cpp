#include "borderbutton.h"
#include <QMouseEvent>
#include <QEvent>
#include <QWidget>
#include <QDebug>
#include <QButtonGroup>
#include <QStyle>
#include "app/global.h"
#include "widgets/util.h"
#include "launcherframe.h"

BorderButton::BorderButton(QWidget *parent) : QPushButton(parent)
{
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("BorderButton");
    setMouseTracking(true);
    setProperty("state", "normal");
    setStyleSheet(getQssFromFile(":/qss/skin/qss/main.qss"));
}

void BorderButton::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::RightButton){
       emit rightClicked(mapToGlobal(event->pos()));
       return;
    }
    QPushButton::mousePressEvent(event);
}

void BorderButton::enterEvent(QEvent *event){
    setChecked(true);
    QPushButton::enterEvent(event);
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
    if (isHightlight){
        setProperty("state", "Highlight");
    }else{
        setProperty("state", "normal");
    }
    updateStyle();
}

void BorderButton::toggleHighlight(){
    setHighlight(!isHighlight());
}

BorderButton::~BorderButton()
{

}

