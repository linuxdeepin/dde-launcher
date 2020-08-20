#ifndef SCROLLWIDGETAGENT_H
#define SCROLLWIDGETAGENT_H

#include <QObject>

#include "blurboxwidget.h"
#include "../global_util/calculate_util.h"


enum PosType {
    Pos_None,
    Pos_LL,
    Pos_L,
    Pos_M,
    Pos_R,
    Pos_RR
 };

enum ScrollType {
    Scroll_Current,
    Scroll_Prev,
    Scroll_Next
};

class ScrollParallelAnimationGroup : public QParallelAnimationGroup
{
    Q_OBJECT

public:
    explicit ScrollParallelAnimationGroup(QObject *parent = nullptr) : QParallelAnimationGroup(parent) {

    }

    ScrollType currentScrollType() {return m_currentScrollType;}
    void setScrollType(ScrollType scrollType) { m_currentScrollType = scrollType;}

private:
    ScrollType m_currentScrollType;
};

class ScrollWidgetAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QPoint pos READ pos WRITE setPos)

public:
    explicit ScrollWidgetAgent(QObject *parent = nullptr);
    ~ScrollWidgetAgent();
    void setControlWidget(QWidget *widget);
    BlurBoxWidget * blurBoxWidget() { return m_blurBoxWidget;}
    void setBlurBoxWidget(BlurBoxWidget * blurBoxWidget);
    QPropertyAnimation *animation() { return m_animation;}
    QPoint pos() { return  m_pos;}
    void setPos(QPoint p);
    PosType posType() { return m_currentPosType;}
    void setPosType(PosType posType);
    void setScrollToType(PosType posType);
    void setVisible(bool visible);
    QPoint getScrollWidgetAgentPos(PosType posType);

Q_SIGNALS:
    void scrollBlurBoxWidget(ScrollWidgetAgent * widgetAgent);
public slots:
    void scrollFinished();

protected:
    void updateBackBlurWidget();
    void updateBackBlurPos();

private:
    QPoint m_pos;
    QPoint m_scrollPos;
    QWidget * m_controlWidget = nullptr;
    QWidget * m_mainWidget = nullptr;
    BlurBoxWidget * m_blurBoxWidget = nullptr;
    QPropertyAnimation *m_animation;
    PosType m_currentPosType;
    PosType m_scrollToType;
    CalculateUtil * m_calcUtil;
};

#endif // SCROLLWIDGETAGENT_H
