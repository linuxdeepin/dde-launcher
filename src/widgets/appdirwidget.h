#ifndef APPDIRWIDGET_H
#define APPDIRWIDGET_H

#include "appitemdelegate.h"
#include "multipagesview.h"

#include <QWidget>
#include <QSharedPointer>

#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class AppDirWidget : public QWidget
{
    Q_OBJECT

public:
    AppDirWidget(QWidget *parent = Q_NULLPTR);
    virtual ~AppDirWidget();

    void initUi();
    void initAccessible();
    void updateBackgroundImage(const QPixmap & img);

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    AppItemDelegate *m_appDelegate;
    MultiPagesView *m_multipageView;

    QSharedPointer<DBlurEffectGroup> m_blurGroup;
    DBlurEffectWidget *m_blurBackground;
    QPixmap m_pix;
};

#endif
