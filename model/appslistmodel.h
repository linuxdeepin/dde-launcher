#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>
#include "dbus/fileInfo_interface.h"
#include "global_util/global.h"

class AppsManager;
class AppsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AppItemRole {
        ItemSizeHintRole = Qt::SizeHintRole,
        AppNameRole = Qt::DisplayRole,
        AppReserveRole = Qt::UserRole,
        AppIconRole = Qt::DecorationRole,
        AppCategoryRole,
        AppAutoStartRole,
        AppNewInstallRole,
    };

    /*enum AppCategory {
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
    };*/

public:
    explicit AppsListModel(CategoryID id, QObject *parent = 0);
    void setListModelData(CategoryID cate);
//    CategoryID getListModeData();
protected:
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    CategoryID m_appCategory = CategoryID::All;
    AppsManager *m_appsManager;

};

#endif // APPSLISTMODEL_H
