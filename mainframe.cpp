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
    m_customAppsModel(new AppsListModel)
{
    m_appsArea->setFrameStyle(QFrame::NoFrame);
    m_customAppsView->setModel(m_customAppsModel);
    m_testView->setModel(m_customAppsModel);
//    m_customAppsView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

//    QVBoxLayout *rightSideLayout = new QVBoxLayout;
//    rightSideLayout->addWidget(m_customAppsView);

    QFrame *frame = new QFrame;
    frame->setFixedSize(200, 20);
    frame->setStyleSheet("background-color:red;");

    m_appsVbox->layout()->addWidget(m_customAppsView);
    m_appsVbox->layout()->addWidget(frame);
    m_appsVbox->layout()->addWidget(m_testView);
    m_appsVbox->layout()->setSpacing(0);
    m_appsVbox->layout()->setMargin(0);
//    QVBoxLayout *la = new QVBoxLayout;
//    la->addWidget(m_customAppsView);
//    m_appsVbox->setLayout(la);
//    m_appsVbox->layout()->addWidget(m_customAppsView);
    m_appsArea->setWidget(m_appsVbox);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_appsArea);

    setMinimumSize(800, 600);
    setLayout(mainLayout);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_customAppsView, &AppListView::doubleClicked, [this] (const QModelIndex &index) {
        m_customAppsModel->removeRow(index.row());
        m_customAppsView->updateGeometry();
        m_customAppsView->adjustSize();
        m_customAppsView->updatea();

//        qDebug() << viewport()->size();
//        qDebug() << contentsSize() << contentsRect();
    //    setFixedSize(contentsSize();
//        m_customAppsView->setFixedWidth(m_customAppsView->contentsRect().width());
//       m_customAppsView-> setFixedHeight(m_customAppsView->contentsSize().height());
    });
}

void MainFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    m_customAppsView->setFixedWidth(m_appsArea->width());
    m_testView->setFixedWidth(m_appsArea->width());
    m_appsVbox->setFixedWidth(m_appsArea->width());
//    m_customAppsView->adjustSize();
//    m_testView->adjustSize()
    m_appsVbox->adjustSize();
}
