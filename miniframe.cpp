#include "miniframe.h"
#include "dbusdock.h"

#include <QRect>
#include <QKeyEvent>
#include <QMouseEvent>

MiniFrame::MiniFrame(QWidget *parent)
    : DBlurEffectWidget(parent),

      m_dockInter(new DBusDock(this))
{
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMaskColor(DBlurEffectWidget::DarkColor);
    setFixedSize(640, 480);
    setBlurRectXRadius(5);
    setBlurRectYRadius(5);

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MiniFrame::adjustPosition, Qt::QueuedConnection);

    QTimer::singleShot(1, this, &MiniFrame::adjustPosition);
}

void MiniFrame::showLauncher()
{
    show();
}

void MiniFrame::hideLauncher()
{
    hide();
}

bool MiniFrame::visible()
{
    return isVisible();
}

void MiniFrame::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton)
        hide();
}

void MiniFrame::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);

    switch (e->key())
    {
    case Qt::Key_Escape:    hide();     break;
    default:;
    }
}

void MiniFrame::showEvent(QShowEvent *e)
{
    DBlurEffectWidget::showEvent(e);

    QTimer::singleShot(1, this, [this] () {
        raise();
        activateWindow();
    });
}

void MiniFrame::adjustPosition()
{
    const QRect dockRect = m_dockInter->frontendRect();
    qDebug() << dockRect;
}
