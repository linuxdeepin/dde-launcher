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

    setLayout(qvLayout);
    qvLayout->setAlignment(Qt::AlignTop);
    setFixedWidth(m_calcUtil->getAppBoxSize().width());
}
