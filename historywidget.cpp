#include "historywidget.h"
#include "model/historymodel.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QProcess>

HistoryWidget::HistoryWidget(QWidget *parent)
    : QWidget(parent),

      m_historyModel(new HistoryModel),
      m_historyView(new QListView)
{
    m_historyView->setModel(m_historyModel);
    m_historyView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_historyView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_historyView);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);

    connect(m_historyView, &QListView::clicked, this, &HistoryWidget::onItemClicked);
}

void HistoryWidget::onItemClicked(const QModelIndex &index)
{
    const QString &fullpath = index.data(HistoryModel::ItemFullPathRole).toString();

    QProcess::startDetached("xdg-open", QStringList() << fullpath);
}
