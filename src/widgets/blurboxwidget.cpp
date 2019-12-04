#include "blurboxwidget.h"
#include "src/global_util/calculate_util.h"


DWIDGET_USE_NAMESPACE
BlurBoxWidget::BlurBoxWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_calcUtil(CalculateUtil::instance())
{
    setMaskColor(DBlurEffectWidget::AutoColor);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    qvLayout = new QVBoxLayout();
    setLayout(qvLayout);
    qvLayout->setAlignment(Qt::AlignTop);
    setFixedSize(m_calcUtil->getAppBoxSize());
}
