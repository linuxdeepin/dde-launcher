#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>

class AppsManager;
class AppsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AppItemRole {
        ItemSizeHintRole = Qt::SizeHintRole,
        AppNameRole = Qt::DisplayRole,
        AppReserveRole = Qt::UserRole,
        AppIconRole,
        AppCategoryRole,
        AppAutoStartRole,
        AppNewInstallRole,
    };

    enum AppCategory {
        All,
        Internet,
        Music,
        Video,
        Graphics,
        Game,
        Office,
        Reading,
        Development,
        System,
        Others,
    };

public:
    explicit AppsListModel(QObject *parent = 0);

protected:
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    AppsManager *m_appsManager;
};

#endif // APPSLISTMODEL_H
