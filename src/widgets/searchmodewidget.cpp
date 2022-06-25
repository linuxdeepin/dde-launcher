#include "searchmodewidget.h"
#include <QVBoxLayout>

SearchModeWidget::SearchModeWidget(QWidget *parent)
    : QWidget (parent)
    , m_nativeLabel(new QLabel(this))
    , m_outsideLabel(new QLabel(this))
    , m_nativeView(new AppGridView(AppGridView::MainView, this))
    , m_outsideView(new AppGridView(AppGridView::MainView, this))
    , m_nativeModel(new AppsListModel(AppsListModel::Search))
    , m_outsideModel(new AppsListModel(AppsListModel::PluginSearch))
    , m_emptyIcon(new QLabel(this))
    , m_emptyText(new QLabel(this))
    , m_nativeWidget(new QWidget(this))
    , m_outsideWidget(new QWidget(this))
    , m_emptyWidget(new QWidget(this))
    , m_launcherInter(new DBusLauncher(this))
{
    initAppView();
    initUi();
    initTitle();
    initConnection();
}

SearchModeWidget::~SearchModeWidget()
{
}

void SearchModeWidget::initUi()
{
    QVBoxLayout *nativeVLayout = new QVBoxLayout;
    nativeVLayout->setContentsMargins(QMargins(0, 10, 0, 0));
    nativeVLayout->setSpacing(10);
    nativeVLayout->addWidget(m_nativeLabel);
    nativeVLayout->addWidget(m_nativeView);

    QHBoxLayout *nativeHLayout = new QHBoxLayout;
    nativeHLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    nativeHLayout->setSpacing(0);

    addSpacerItem(nativeHLayout);
    nativeHLayout->addLayout(nativeVLayout);
    nativeHLayout->setStretch(1, 1);
    addSpacerItem(nativeHLayout);

    QVBoxLayout *outsideVLayout = new QVBoxLayout;
    outsideVLayout->setContentsMargins(QMargins(0, 10, 0, 0));
    outsideVLayout->setSpacing(10);
    outsideVLayout->addWidget(m_outsideLabel);
    outsideVLayout->addWidget(m_outsideView);

    QHBoxLayout *outsideHLayout = new QHBoxLayout;
    outsideHLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    outsideHLayout->setSpacing(0);

    addSpacerItem(outsideHLayout);
    outsideHLayout->addLayout(outsideVLayout);
    outsideHLayout->setStretch(1, 1);
    addSpacerItem(outsideHLayout);

    // 搜索为空时
    QVBoxLayout *emptyVLayout  = new QVBoxLayout;
    emptyVLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    emptyVLayout->setSpacing(0);
    emptyVLayout->addWidget(m_emptyIcon);
    emptyVLayout->addWidget(m_emptyText);

    m_nativeWidget->setLayout(nativeHLayout);
    m_outsideWidget->setLayout(outsideHLayout);
    m_emptyWidget->setLayout(emptyVLayout);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(QMargins(10, 10 ,10, 0));
    mainVLayout->addWidget(m_nativeWidget);
    mainVLayout->addWidget(m_outsideWidget);
    mainVLayout->addWidget(m_emptyWidget, 0, Qt::AlignCenter);
    mainVLayout->setStretch(0, 1);
    mainVLayout->setStretch(1, 1);
    mainVLayout->setStretch(2, 2);

    setLayout(mainVLayout);

    m_emptyWidget->setVisible(false);
}

void SearchModeWidget::initTitle()
{
    QPalette pal = QPalette();
    if (CalculateUtil::instance()->fullscreen())
        pal.setColor(QPalette::WindowText, Qt::white);
    else
        pal.setColor(QPalette::WindowText, Qt::black);

    m_nativeLabel->setPalette(pal);
    m_outsideLabel->setPalette(pal);
    m_emptyText->setPalette(pal);
}

void SearchModeWidget::initAppView()
{
    QPalette nativePal = m_nativeLabel->palette();
    nativePal.setColor(QPalette::WindowText,Qt::white);
    m_nativeLabel->setPalette(nativePal);

    QPalette outsidePal = m_outsideLabel->palette();
    outsidePal.setColor(QPalette::WindowText,Qt::white);
    m_outsideLabel->setPalette(outsidePal);

    m_nativeView->setFlow(QListView::LeftToRight);
    m_outsideView->setFlow(QListView::LeftToRight);

    m_nativeView->setWrapping(true);
    m_outsideView->setWrapping(true);

    m_nativeView->setModel(m_nativeModel);
    m_outsideView->setModel(m_outsideModel);
}

void SearchModeWidget::initConnection()
{
    QMetaObject::invokeMethod(this, "connectViewEvent", Qt::QueuedConnection, Q_ARG(AppGridView *, m_nativeView));
    QMetaObject::invokeMethod(this, "connectViewEvent", Qt::QueuedConnection, Q_ARG(AppGridView *, m_outsideView));
}

void SearchModeWidget::setItemDelegate(AppItemDelegate *delegate)
{
    m_nativeView->setItemDelegate(delegate);
    m_outsideView->setItemDelegate(delegate);
}

void SearchModeWidget::setSearchModel(QSortFilterProxyModel *model)
{
    updateTitleContent();
    updateTitlePos(m_launcherInter->fullscreen());

    m_nativeView->setModel(model);
    m_nativeWidget->setVisible(model->rowCount(QModelIndex()) > 0);
    m_outsideWidget->setVisible(m_outsideModel->rowCount(QModelIndex()) > 0);
    m_emptyWidget->setVisible(model->rowCount(QModelIndex()) <= 0);
}

void SearchModeWidget::updateTitleContent()
{
    m_nativeLabel->setText(tr("Your searched apps in local:"));

    ItemInfoList_v1 list = AppsManager::instance()->appsInfoList(AppsListModel::PluginSearch);
    ItemInfo_v1 info;
    if (list.size() > 0)
        info = list.at(0);

    m_outsideLabel->setText(info.m_description);
}

void SearchModeWidget::updateTitlePos(bool alignCenter)
{
    if (alignCenter) {
        m_nativeLabel->setAlignment(Qt::AlignCenter);
        m_outsideLabel->setAlignment(Qt::AlignCenter);
    } else {
        m_nativeLabel->setAlignment(Qt::AlignLeft);
        m_outsideLabel->setAlignment(Qt::AlignLeft);
    }
}

void SearchModeWidget::addSpacerItem(QBoxLayout *layout)
{
    if (m_launcherInter->fullscreen())
        layout->addSpacerItem(new QSpacerItem(300, 5, QSizePolicy::Expanding, QSizePolicy::Expanding));
    else
        layout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Expanding));
}
