#include "blurboxwidget.h"
#include "src/global_util/calculate_util.h"
#include <QPainter>

DWIDGET_USE_NAMESPACE
BlurBoxWidget::BlurBoxWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_vLayout(new QVBoxLayout(this))
    , m_maskLayer(new QWidget(this))
    , m_calcUtil(CalculateUtil::instance())
{
    setMaskColor(LightColor);
    setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    setFixedWidth(m_calcUtil->getAppBoxSize().width());

    setLayout(m_vLayout);
    m_vLayout->setContentsMargins(60, 27, 30, 0);
    m_vLayout->setAlignment(Qt::AlignTop);
}

void BlurBoxWidget::layoutAddWidget(QWidget *child)
{
    m_vLayout->addWidget(child);
}

void BlurBoxWidget::layoutAddWidget(QWidget *child, int stretch, Qt::Alignment alignment)
{
    m_vLayout->addWidget(child, stretch, alignment);
}

void BlurBoxWidget::paintEvent(QPaintEvent *event)
{
    DBlurEffectWidget::paintEvent(event);
    if (m_maskLayer) {
        QPainter painter(this);
        m_maskLayer->raise();
        painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter.setBrush(QBrush(QColor(0, 0, 0, 100)));
        painter.setPen(Qt::transparent);
        QRect rect = this->rect();
        rect.setWidth(rect.width() - 1);
        rect.setHeight(rect.height() - 1);
        painter.drawRoundedRect(rect, DLauncher::APPHBOX_RADIUS, DLauncher::APPHBOX_RADIUS);
        QWidget::paintEvent(event);
    }
}

void BlurBoxWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        mousePos = e->pos();
    }
}

void BlurBoxWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && e->pos() == mousePos) {
        int nNext = 0;
        if (m_calcUtil->getScreenSize().width() / 2 < e->globalX()) {
            nNext = 1;
        } else {
            nNext = -1;
        }
        emit maskClick(getCategory(), nNext);
    }
}

void BlurBoxWidget::setMaskSize(QSize size)
{
    m_maskLayer->setFixedSize(size);
    m_maskLayer->raise();
}

void BlurBoxWidget::setMaskVisible(bool visible)
{
    m_maskLayer->setVisible(visible);
}


