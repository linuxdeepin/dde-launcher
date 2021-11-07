#include "scrollwidgetagent.h"

ScrollWidgetAgent::ScrollWidgetAgent(QObject *parent)
    : QObject(parent)
    , m_pos(QPoint(0,0))
    , m_controlWidget(nullptr)
    , m_mainWidget(nullptr)
    , m_blurBoxWidget(nullptr)
    , m_animation(new QPropertyAnimation(this,"pos", this))
    , m_currentPosType(Pos_None)
    , m_scrollToType(Pos_None)
{
    m_mainWidget = static_cast<QWidget *>(parent);

    m_animation->setDuration(200);

    connect(m_animation, &QPropertyAnimation::finished, this, &ScrollWidgetAgent::scrollFinished);
}

ScrollWidgetAgent::~ScrollWidgetAgent()
{

}

void ScrollWidgetAgent::setControlWidget(QWidget *widget)
{
    m_controlWidget = widget;
}

void ScrollWidgetAgent::setBlurBoxWidget(BlurBoxWidget *blurBoxWidget)
{
    if (m_blurBoxWidget != blurBoxWidget){
        m_blurBoxWidget = blurBoxWidget;
        updateBackBlurWidget();
        if (m_blurBoxWidget) {
            m_blurBoxWidget->move(m_pos);
            updateBackBlurPos();
        }
    }
}

void ScrollWidgetAgent::setPos(QPoint p)
{
    m_pos = p;
    if (m_blurBoxWidget) {
        m_blurBoxWidget->move(m_pos);
        updateBackBlurPos();
    }
    emit scrollBlurBoxWidget(this);
}

void ScrollWidgetAgent::setVisible(bool visible)
{
    if (m_blurBoxWidget) {
        m_blurBoxWidget->setVisible(visible);
        updateBackBlurPos();
    }
}

void ScrollWidgetAgent::setPosType(PosType posType)
{
    m_currentPosType = posType;
    updateBackBlurWidget();
}

void ScrollWidgetAgent::setScrollToType(PosType posType)
{
    m_scrollToType = posType;
    m_scrollPos = getScrollWidgetAgentPos(m_scrollToType);
    m_animation->setEndValue(m_scrollPos);
}

void ScrollWidgetAgent::scrollFinished()
{
    m_currentPosType = m_scrollToType;
    updateBackBlurWidget();
    m_pos = m_scrollPos;
}

void ScrollWidgetAgent::updateBackBlurWidget()
{
    if (m_blurBoxWidget) {
        switch (m_currentPosType) {
        case Pos_LL:
            m_blurBoxWidget->setMaskVisible(true);
            m_blurBoxWidget->setBlurBgVisible(true);
            break;
        case Pos_L:
            m_blurBoxWidget->setMaskVisible(true);
            m_blurBoxWidget->setBlurBgVisible(true);
            break;
        case Pos_M:
            m_blurBoxWidget->setMaskVisible(false);
            m_blurBoxWidget->setBlurBgVisible(true);
            break;
        case Pos_R:
            m_blurBoxWidget->setMaskVisible(true);
            m_blurBoxWidget->setBlurBgVisible(true);
            break;
        case Pos_RR:
            m_blurBoxWidget->setMaskVisible(true);
            m_blurBoxWidget->setBlurBgVisible(true);
            break;
        default: break;
        }

        m_blurBoxWidget->update();
    }
}

void ScrollWidgetAgent::updateBackBlurPos()
{
    if (m_mainWidget && m_controlWidget && m_blurBoxWidget->isVisible()) {
        QPoint p = m_controlWidget->mapTo(m_mainWidget->window(), m_pos);
        m_blurBoxWidget->updateBackBlurPos(p);
        m_blurBoxWidget->categoryTitle()->updatePosition(m_pos, m_controlWidget->width(), m_currentPosType);
        m_blurBoxWidget->update();
    }
}

QPoint ScrollWidgetAgent::getScrollWidgetAgentPos(PosType posType)
{
    QPoint pos = QPoint(0,0);

    if (!m_controlWidget) {
        return pos;
    }

    int boxWidth = CalculateUtil::instance()->getAppBoxSize().width();

    pos.setY(0);
    switch (posType) {
    case Pos_LL:
        pos.setX(m_controlWidget->width() / 2 - boxWidth / 2 - boxWidth * 2 - DLauncher::APPHBOX_SPACING *  2);
        break;
    case Pos_L:
        pos.setX(m_controlWidget->width() / 2 - boxWidth / 2 - boxWidth - DLauncher::APPHBOX_SPACING);
        break;
    case Pos_M:
        pos.setX(m_controlWidget->width() / 2 - boxWidth / 2);
        break;
    case Pos_R:
        pos.setX(m_controlWidget->width() / 2 - boxWidth / 2 + boxWidth + DLauncher::APPHBOX_SPACING);
        break;
    case Pos_RR:
        pos.setX(m_controlWidget->width() / 2 - boxWidth / 2 + boxWidth * 2 + DLauncher::APPHBOX_SPACING * 2);
        break;
    case Pos_None:
        pos.setX(0);
        break;
    }

    return pos;
}
