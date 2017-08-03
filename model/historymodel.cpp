#include "historymodel.h"

#include <QTimer>
#include <QDebug>
#include <QUrl>

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QTimer::singleShot(1, this, &HistoryModel::updateHistory);
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_data.size();
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
    case ItemNameRole:
        return m_data[index.row()].fileName();
    case ItemFullPathRole:
        return m_data[index.row()].absoluteFilePath();
    default:;
    }

    return QVariant();
}

void HistoryModel::updateHistory()
{
    const QStringList history = m_history.history();

    for (const auto &h : history)
    {
        const QFileInfo info(QUrl(h).toLocalFile());
        if (!info.exists())
            continue;

        m_data << info;
    }
}
