#include "appdrawerwidget.h"
#include "calculate_util.h"
#include "appsmanager.h"
#include "constants.h"

#include <QHBoxLayout>

AppDrawerWidget::AppDrawerWidget(QWidget *parent)
    : QWidget (parent)
    , m_appDelegate(new AppItemDelegate(this))
    , m_multipageView(new MultiPagesView(AppsListModel::Dir, this))
    , m_blurGroup(QSharedPointer<DBlurEffectGroup>(new DBlurEffectGroup))
    , m_blurBackground(new DBlurEffectWidget(this))
{
    initUi();
    initAccessible();
}

AppDrawerWidget::~AppDrawerWidget()
{
}

void AppDrawerWidget::initUi()
{
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);

    m_multipageView->setDataDelegate(m_appDelegate);

    // TODO: 标题命名待优化．
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_multipageView);
    setLayout(mainLayout);

    m_blurBackground->setAccessibleName(QString("blurBackground"));
    m_blurBackground->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurBackground->setMaskAlpha(30);
    m_blurBackground->setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    m_blurBackground->lower();
    m_blurGroup->addWidget(m_blurBackground);
    m_multipageView->raise();
}

void AppDrawerWidget::initAccessible()
{
    m_multipageView->setAccessibleName("drawerWidget");
}

void AppDrawerWidget::updateBackgroundImage(const QPixmap &img)
{
    m_pix = img;
    update();
}

void AppDrawerWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    QSize itemSize = CalculateUtil::instance()->appItemSize() * 5 / 4;
    QSize widgetSize = QSize(itemSize.width() * 4, itemSize.height() * 3 + 60);
    setFixedSize(widgetSize);
    m_multipageView->setFixedSize(widgetSize);
    m_blurBackground->setFixedSize(widgetSize);
    m_blurGroup->setSourceImage(m_pix.toImage(), 0);

    QPoint center = AppsManager::instance()->currentScreen()->geometry().center() - QPoint(size().width() / 2, size().height() / 2);
    setGeometry(QRect(center, size()));

    m_multipageView->updatePageCount(AppsListModel::Dir);
    m_multipageView->updatePosition(5);
    m_multipageView->setModel(AppsListModel::Dir);

    // TODO: 更新并显示标题名称
    m_multipageView->refreshTitle("");
}
