#include "miniframe.h"
#include "dbusdock.h"
#include "widgets/miniframenavigation.h"
#include "widgets/searchlineedit.h"

#include <QRect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHBoxLayout>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

MiniFrame::MiniFrame(QWidget *parent)
    : DBlurEffectWidget(parent),

      m_dockInter(new DBusDock(this)),

      m_navigation(new MiniFrameNavigation)
{
    m_navigation->setFixedWidth(140);

    m_viewToggle = new DImageButton;
    m_modeToggle = new DImageButton;
    m_searchEdit = new SearchLineEdit;

    QHBoxLayout *viewHeaderLayout = new QHBoxLayout;
    viewHeaderLayout->addWidget(m_viewToggle);
    viewHeaderLayout->addStretch();
    viewHeaderLayout->addWidget(m_searchEdit);
    viewHeaderLayout->addStretch();
    viewHeaderLayout->addWidget(m_modeToggle);
    viewHeaderLayout->setSpacing(0);
    viewHeaderLayout->setMargin(0);

    QVBoxLayout *viewLayout = new QVBoxLayout;
    viewLayout->addLayout(viewHeaderLayout);
    viewLayout->addStretch();
    viewLayout->setSpacing(0);
    viewLayout->setMargin(0);

    m_viewWrapper = new QWidget;
    m_viewWrapper->setLayout(viewLayout);
    m_viewWrapper->setStyleSheet("background-color: rgba(255, 255, 255, .1); border-radius: 8px;");

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addWidget(m_navigation);
    centralLayout->addWidget(m_viewWrapper);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 10, 10, 10);

    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMaskColor(DBlurEffectWidget::DarkColor);
    setFixedSize(640, 480);
    setBlurRectXRadius(5);
    setBlurRectYRadius(5);
    setLayout(centralLayout);
}

void MiniFrame::showLauncher()
{
    if (visible())
        return;

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MiniFrame::adjustPosition, Qt::QueuedConnection);
    QTimer::singleShot(1, this, &MiniFrame::adjustPosition);

    show();
}

void MiniFrame::hideLauncher()
{
    if (!visible())
        return;

    disconnect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MiniFrame::adjustPosition);

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
    case Qt::Key_Escape:    hideLauncher();     break;
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
    const int dockPos = m_dockInter->position();
    const QRect dockRect = m_dockInter->frontendRect();

    const int spacing = 10;
    const QSize s = size();
    QPoint p;

    switch (dockPos)
    {
    case DOCK_TOP:
        p = QPoint(dockRect.left(), dockRect.bottom() + spacing);
        break;
    case DOCK_BOTTOM:
        p = QPoint(dockRect.left(), dockRect.top() - s.height() - spacing);
        break;
    case DOCK_LEFT:
        p = QPoint(dockRect.right() + spacing, dockRect.top());
        break;
    case DOCK_RIGHT:
        p = QPoint(dockRect.left() - s.width() - spacing, dockRect.top());
        break;
    default: Q_UNREACHABLE_IMPL();
    }

    move(p);
}
