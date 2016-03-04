#include "mainframe.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>

MainFrame::MainFrame(QWidget *parent) :
    QFrame(parent),
    m_appsArea(new QScrollArea),
    m_appsVbox(new DVBoxWidget),
    m_customAppsView(new AppListView),
    m_testView(new AppListView),
    m_customAppsModel(new AppsListModel),
    m_appItemDelegate(new AppItemDelegate)
{
    m_appsVbox->installEventFilter(this);
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_customAppsView->setModel(m_customAppsModel);
    m_customAppsView->setItemDelegate(m_appItemDelegate);
    m_customAppsView->setStyleSheet("background-color:cyan;");
    m_testView->setModel(m_customAppsModel);
    m_testView->setItemDelegate(m_appItemDelegate);
    m_testView->setStyleSheet("background-color:orange;");

    QFrame *frame = new QFrame;
    frame->setFixedSize(200, 20);
    frame->setStyleSheet("background-color:red;");

    m_appsVbox->layout()->addWidget(m_customAppsView);
    m_appsVbox->layout()->addWidget(frame);
    m_appsVbox->layout()->addWidget(m_testView);
    m_appsVbox->layout()->setSpacing(0);
    m_appsVbox->layout()->setMargin(0);
    m_appsArea->setWidget(m_appsVbox);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_appsArea);

    setMinimumSize(800, 600);
    setLayout(mainLayout);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_customAppsView, &AppListView::doubleClicked, [this] (const QModelIndex &index) {
        m_customAppsModel->removeRow(index.row());
    });
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    const int appsContentWidth = m_appsArea->width();

    m_appsVbox->setFixedWidth(appsContentWidth);
    m_customAppsView->setFixedWidth(appsContentWidth);
    m_testView->setFixedWidth(appsContentWidth);
}

bool MainFrame::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    Q_UNUSED(e);

    return false;
}
