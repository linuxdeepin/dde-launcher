#include "mainframe.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QHBoxLayout>
#include <QDebug>

MainFrame::MainFrame(QWidget *parent) :
    QFrame(parent),
    m_customAppsView(new AppListView),
    m_customAppsModel(new AppsListModel)
{
    m_customAppsView->setModel(m_customAppsModel);

    QVBoxLayout *rightSideLayout = new QVBoxLayout;
    rightSideLayout->addWidget(m_customAppsView);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(rightSideLayout);

    setLayout(mainLayout);
    setMinimumSize(800, 600);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_customAppsView, &AppListView::doubleClicked, [this] (const QModelIndex &index) {
        m_customAppsModel->removeRow(index.row());
    });
}
