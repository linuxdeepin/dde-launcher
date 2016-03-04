#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include "appsmanager.h"

#include <QAbstractListModel>

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
    };

public:
    explicit AppsListModel(QObject *parent = 0);

    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

protected:
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;

private:
    AppsManager *m_appsManager;
};

#endif // APPSLISTMODEL_H
