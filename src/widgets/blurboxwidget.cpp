#include "blurboxwidget.h"
#include "src/global_util/calculate_util.h"

DWIDGET_USE_NAMESPACE
BlurBoxWidget::BlurBoxWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , qvLayout(new QVBoxLayout(this))
    , m_maskLayer(new QWidget(this))
    , m_calcUtil(CalculateUtil::instance())
{
    setMaskColor(DBlurEffectWidget::AutoColor);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    setFixedWidth(m_calcUtil->getAppBoxSize().width());

//    QPalette pal(m_maskLayer->palette());
//    pal.setColor(QPalette::Background, QColor(0x0, 0x0, 0x0, 10));
//    m_maskLayer->setAutoFillBackground(true);
//    m_maskLayer->setPalette(pal);
    m_maskLayer->show();

    setLayout(qvLayout);
    qvLayout->setSpacing(10);
    qvLayout->setAlignment(Qt::AlignTop);

    initconnect();
}

void BlurBoxWidget::initconnect()
{

}

void BlurBoxWidget::layoutAddWidget(QWidget *child)
{
    qvLayout->addWidget(child);
}

void BlurBoxWidget::layoutAddWidget(QWidget *child, int stretch, Qt::Alignment alignment)
{
    qvLayout->addWidget(child, stretch, alignment);
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


