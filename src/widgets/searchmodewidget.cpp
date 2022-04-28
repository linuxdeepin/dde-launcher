#include "searchmodewidget.h"
#include <QVBoxLayout>

SearchModeWidget::SearchModeWidget(QWidget *parent)
    : QWidget (parent)
    , m_nativeLabel(new QLabel(this))
    , m_outsideLabel(new QLabel(this))
    , m_nativeView(new AppGridView(this))
    , m_outsideView(new AppGridView(this))
    , m_nativeModel(new AppsListModel(AppsListModel::Search))
    , m_outsideModel(new AppsListModel(AppsListModel::PluginSearch))
    , m_emptyIcon(new QLabel(this))
    , m_emptyText(new QLabel(this))
    , m_nativeWidget(new QWidget(this))
    , m_outsideWidget(new QWidget(this))
    , m_emptyWidget(new QWidget(this))
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
    m_nativeView->setFixedSize(600, 400);
    m_outsideView->setFixedSize(600, 400);

//    m_nativeView->setStyleSheet("QListView{border: 1px solid red;}");
//    m_outsideView->setStyleSheet("QListView{border: 1px solid red;}");

    QVBoxLayout *nativeVLayout = new QVBoxLayout;
    nativeVLayout->setContentsMargins(QMargins(0, 10, 0, 0));
    nativeVLayout->setSpacing(10);
    nativeVLayout->addWidget(m_nativeLabel);
    nativeVLayout->addWidget(m_nativeView);

    QHBoxLayout *nativeHLayout = new QHBoxLayout;
    nativeHLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    nativeHLayout->setSpacing(0);

    nativeHLayout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Preferred));
    nativeHLayout->addLayout(nativeVLayout);
    nativeHLayout->setStretch(1, 1);
    nativeHLayout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_nativeWidget->setLayout(nativeHLayout);

    QVBoxLayout *outsideVLayout = new QVBoxLayout;
    outsideVLayout->setContentsMargins(QMargins(0, 10, 0, 0));
    outsideVLayout->setSpacing(10);
    outsideVLayout->addWidget(m_outsideLabel);
    outsideVLayout->addWidget(m_outsideView);

    QHBoxLayout *outsideHLayout = new QHBoxLayout;
    outsideHLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    outsideHLayout->setSpacing(0);

    outsideHLayout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Preferred, QSizePolicy::Preferred));
    outsideHLayout->addLayout(outsideVLayout);
    outsideHLayout->addSpacerItem(new QSpacerItem(5, 5, QSizePolicy::Preferred, QSizePolicy::Preferred));
    m_outsideWidget->setLayout(outsideHLayout);

    // 搜索为空时
    QVBoxLayout *emptyVLayout  = new QVBoxLayout;
    emptyVLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    emptyVLayout->setSpacing(0);
    emptyVLayout->addWidget(m_emptyIcon);
    emptyVLayout->addWidget(m_emptyText);
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
    m_nativeLabel->setText(tr("Native Result:"));
    m_nativeLabel->setAlignment(Qt::AlignHCenter);
    m_outsideLabel->setText(tr("Other Result:"));
    m_outsideLabel->setAlignment(Qt::AlignHCenter);

    m_emptyText->setText(tr("No Search Result"));
    QPalette pal = m_emptyText->palette();
    pal.setColor(QPalette::WindowText, Qt::white);
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

    m_nativeView->setViewType(AppGridView::PopupView);
    m_outsideView->setViewType(AppGridView::PopupView);

    m_nativeView->setFlow(QListView::LeftToRight);
    m_outsideView->setFlow(QListView::LeftToRight);

    m_nativeView->setWrapping(true);
    m_outsideView->setWrapping(false);

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
    m_nativeView->setModel(model);
    m_nativeWidget->setVisible(model->rowCount(QModelIndex()) > 0);
    m_outsideWidget->setVisible(m_outsideModel->rowCount(QModelIndex()) > 0);
    m_emptyWidget->setVisible(model->rowCount(QModelIndex()) <= 0);
}
