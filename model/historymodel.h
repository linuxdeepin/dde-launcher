#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include "../global_util/recentlyused.h"

#include <QAbstractListModel>
#include <QFileInfo>

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum HistoryItemRole
    {
        ItemNameRole = Qt::DisplayRole,
        UnusedRole = Qt::UserRole,
        ItemFullPathRole,
    };

    explicit HistoryModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void clear();

private slots:
    void updateHistory();

private:
    RecentlyUsed m_history;
    QList<QFileInfo> m_data;
};

#endif // HISTORYMODEL_H
