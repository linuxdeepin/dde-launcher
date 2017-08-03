#include "historywidget.h"
#include "model/historymodel.h"

#include <QVBoxLayout>

HistoryWidget::HistoryWidget(QWidget *parent)
    : QWidget(parent),

      m_historyView(new QListView)
{
    m_historyView->setModel(new HistoryModel);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_historyView);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);
}
