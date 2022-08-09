#include "searchmodewidget.h"

#include <DFontSizeManager>

#include <QVBoxLayout>

SearchModeWidget::SearchModeWidget(QWidget *parent)
    : QWidget (parent)
    , m_nativeWidget(new QWidget(this))
    , m_outsideWidget(new QWidget(this))
    , m_emptyWidget(new QWidget(this))
    , m_nativeLabel(new QLabel(m_nativeWidget))
    , m_outsideLabel(new QLabel(m_outsideWidget))
    , m_nativeView(new AppGridView(AppGridView::MainView, this))
    , m_outsideView(new AppGridView(AppGridView::MainView, this))
    , m_nativeModel(new AppsListModel(AppsListModel::Search))
    , m_outsideModel(new AppsListModel(AppsListModel::PluginSearch))
    , m_iconButton(new DIconButton(m_emptyWidget))
    , m_emptyText(new QLabel(m_emptyWidget))
    , m_launcherInter(new DBusLauncher(this))
{
    initAppView();
    initTitle();
    initUi();
    initConnection();
}

SearchModeWidget::~SearchModeWidget()
{
}

void SearchModeWidget::initUi()
{
    QMargins contentMargin(0, 0, 0, 0);

    // native app
    QVBoxLayout *nativeVLayout = new QVBoxLayout;
    nativeVLayout->setContentsMargins(contentMargin);
    nativeVLayout->setSpacing(10);
    nativeVLayout->addWidget(m_nativeLabel);
    nativeVLayout->addWidget(m_nativeView);

    QHBoxLayout *nativeHLayout = new QHBoxLayout;
    nativeHLayout->setContentsMargins(contentMargin);
    nativeHLayout->setSpacing(0);

    addSpacerItem(nativeHLayout);
    nativeHLayout->addLayout(nativeVLayout);
    nativeHLayout->setStretch(1, 1);
    addSpacerItem(nativeHLayout);

    // app store app
    QVBoxLayout *outsideVLayout = new QVBoxLayout;
    outsideVLayout->setContentsMargins(contentMargin);
    outsideVLayout->setSpacing(10);
    outsideVLayout->addWidget(m_outsideLabel);
    outsideVLayout->addWidget(m_outsideView);

    QHBoxLayout *outsideHLayout = new QHBoxLayout;
    outsideHLayout->setContentsMargins(contentMargin);
    outsideHLayout->setSpacing(0);

    addSpacerItem(outsideHLayout);
    outsideHLayout->addLayout(outsideVLayout);
    outsideHLayout->setStretch(1, 1);
    addSpacerItem(outsideHLayout);

    // 搜索为空时
    QVBoxLayout *emptyVLayout  = new QVBoxLayout;
    emptyVLayout->setContentsMargins(contentMargin);
    emptyVLayout->setSpacing(0);
    emptyVLayout->addStretch();
    emptyVLayout->addWidget(m_iconButton);
    emptyVLayout->addWidget(m_emptyText);
    emptyVLayout->addStretch();

    m_nativeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outsideWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_emptyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 把他们放在一起,方便处理显示和隐藏
    m_nativeWidget->setLayout(nativeHLayout);
    m_outsideWidget->setLayout(outsideHLayout);
    m_emptyWidget->setLayout(emptyVLayout);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(QMargins(0, 10, 0, 0));
    mainVLayout->addWidget(m_nativeWidget);
    mainVLayout->addWidget(m_outsideWidget);
    mainVLayout->addWidget(m_emptyWidget, 0, Qt::AlignCenter);
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

    // 搜索标题字体样式设置
    QFont searchTitleFont = m_nativeLabel->font();
    DFontSizeManager::instance()->bind(m_nativeLabel, DFontSizeManager::T5);
    searchTitleFont.setBold(true);
    searchTitleFont.setWeight(700);

    m_nativeLabel->setFont(searchTitleFont);
    m_outsideLabel->setFont(searchTitleFont);

    // 搜索为空格的字体样式设置
    QFont noResultTitleFont = m_emptyText->font();
    noResultTitleFont.setBold(true);
    noResultTitleFont.setWeight(700);

    DFontSizeManager::instance()->bind(m_emptyText, DFontSizeManager::T5);
    m_emptyText->setFont(noResultTitleFont);
    m_emptyText->setWindowOpacity(0.3);

    m_iconButton->setIcon(DDciIcon::fromTheme("search_no_result"));
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setIconSize(QSize(128, 128));
    m_iconButton->setEnabled(false);
    m_iconButton->setFlat(true);

    m_emptyText->setText(tr("No search results"));
}

void SearchModeWidget::initAppView()
{
    QPalette nativePal = m_nativeLabel->palette();
    nativePal.setColor(QPalette::WindowText,Qt::white);
    m_nativeLabel->setPalette(nativePal);

    QPalette outsidePal = m_outsideLabel->palette();
    outsidePal.setColor(QPalette::WindowText,Qt::white);
    m_outsideLabel->setPalette(outsidePal);

    AppItemDelegate *itemDelegate = new AppItemDelegate(this);

    m_nativeView->setFlow(QListView::LeftToRight);
    m_nativeView->setWrapping(true);
    m_nativeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_nativeView->setSizeAdjustPolicy(QListView::AdjustToContents);
    m_nativeView->setModel(m_nativeModel);
    m_nativeView->setItemDelegate(itemDelegate);
    m_nativeView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_outsideView->setFlow(QListView::LeftToRight);
    m_outsideView->setWrapping(true);
    m_outsideView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_outsideView->setSizeAdjustPolicy(QListView::AdjustToContents);
    m_outsideView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_outsideView->setModel(m_outsideModel);
    m_outsideView->setItemDelegate(itemDelegate);
}

void SearchModeWidget::initConnection()
{
    connect(CalculateUtil::instance(), &CalculateUtil::layoutChanged, this, &SearchModeWidget::onLayoutChanged);
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

void SearchModeWidget::onLayoutChanged()
{
    int itemSpacing = CalculateUtil::instance()->appItemSpacing();
    QMargins margin(0, 0, 0, 0);
    m_nativeView->setSpacing(itemSpacing);
    m_nativeView->setViewportMargins(margin);
    m_outsideView->setSpacing(itemSpacing);
    m_outsideView->setViewportMargins(margin);
}
