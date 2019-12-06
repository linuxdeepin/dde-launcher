#include "blurboxwidget.h"
#include "src/global_util/calculate_util.h"


DWIDGET_USE_NAMESPACE
BlurBoxWidget::BlurBoxWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , qvLayout(new QVBoxLayout)
    , m_calcUtil(CalculateUtil::instance())
{
    setMaskColor(DBlurEffectWidget::AutoColor);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    setFixedWidth(m_calcUtil->getAppBoxSize().width());

    setLayout(qvLayout);
    qvLayout->setSpacing(10);
    qvLayout->setAlignment(Qt::AlignTop);
    initconnect();
}

void BlurBoxWidget::initconnect()
{
    //connect()

}

void BlurBoxWidget::layoutAddWidget(QWidget *child)
{
    qvLayout->addWidget(child);
}

void BlurBoxWidget::layoutAddWidget(QWidget *child, int stretch, Qt::Alignment alignment)
{
    qvLayout->addWidget(child, stretch, alignment);
}


