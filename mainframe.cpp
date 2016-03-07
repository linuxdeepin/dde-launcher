
#include "mainframe.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QKeyEvent>

MainFrame::MainFrame(QWidget *parent) :
    QFrame(parent),
    m_categoryListWidget(new CategoryListWidget),
    m_searchWidget(new SearchWidget),
    m_appsArea(new QScrollArea),
    m_appsVbox(new DVBoxWidget),
    m_customAppsView(new AppListView),
    m_testView(new AppListView),
    m_customAppsModel(new AppsListModel),
    m_appItemDelegate(new AppItemDelegate),
    m_internetTitle(new CategoryTitleWidget(tr("Internet"))),
    m_musicTitle(new CategoryTitleWidget(tr("Music")))
{
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_appsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_customAppsView->setModel(m_customAppsModel);
    m_customAppsView->setItemDelegate(m_appItemDelegate);
    m_customAppsView->setStyleSheet("background-color:cyan;");
    m_testView->setModel(m_customAppsModel);
    m_testView->setItemDelegate(m_appItemDelegate);
    m_testView->setStyleSheet("background-color:orange;");

    m_appsVbox->layout()->addWidget(m_customAppsView);
    m_appsVbox->layout()->addWidget(m_internetTitle);
    m_appsVbox->layout()->addWidget(m_testView);
    m_appsVbox->layout()->addWidget(m_musicTitle);
    m_appsVbox->layout()->setSpacing(0);
    m_appsVbox->layout()->setMargin(0);
    m_appsArea->setWidget(m_appsVbox);

    QVBoxLayout *rightSideLayout = new QVBoxLayout;
    rightSideLayout->addWidget(m_searchWidget);
    rightSideLayout->addWidget(m_appsArea);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_categoryListWidget);
    mainLayout->addLayout(rightSideLayout);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    setMinimumSize(800, 600);
    setLayout(mainLayout);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_customAppsView, &AppListView::doubleClicked, [this] (const QModelIndex &index) {
        m_customAppsModel->removeRow(index.row());
    });
}

void MainFrame::scrollToCategory(const AppsListModel::AppCategory &category)
{
    QWidget *dest = nullptr;

    switch (category)
    {
    case AppsListModel::Internet:   dest = m_internetTitle;     break;
    default:;
    }

    // scroll to destination
    if (dest)
        m_appsArea->verticalScrollBar()->setValue(dest->pos().y());
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    const int appsContentWidth = m_appsArea->width();

    m_appsVbox->setFixedWidth(appsContentWidth);
    m_customAppsView->setFixedWidth(appsContentWidth);
    m_testView->setFixedWidth(appsContentWidth);
}

void MainFrame::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
    case Qt::Key_Control:       scrollToCategory(AppsListModel::Internet);       break;
#endif
    default:;
    }
}

bool MainFrame::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    Q_UNUSED(e);

    return false;
}
