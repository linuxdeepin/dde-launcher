#ifndef BLURBOXWIDGET_H
#define BLURBOXWIDGET_H

#include <dblureffectwidget.h>
#include <QVBoxLayout>
#include "src/global_util/constants.h"
#include  "src/global_util/calculate_util.h"
#include "qapplication.h"

DWIDGET_USE_NAMESPACE

class BlurBoxWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit BlurBoxWidget(QWidget *parent = nullptr);
    QVBoxLayout *qvLayout ;
public :
    void initconnect();
    void layoutAddWidget(QWidget *child);
    void layoutAddWidget(QWidget *, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

private:
    CalculateUtil *m_calcUtil;
};

#endif // BLURBOXWIDGET_H
