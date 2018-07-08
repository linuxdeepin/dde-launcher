/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>

#define MAXIMUM_POPULAR_ITEMS 11

class AppsManager;
class CalculateUtil;
class AppsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AppItemRole {
        ItemSizeHintRole = Qt::SizeHintRole,
        AppNameRole = Qt::DisplayRole,
        AppIconRole = Qt::DecorationRole,
        AppReserveRole = Qt::UserRole,

        // custom role start
        AppRawItemInfoRole,
        AppDialogIconRole,
        AppKeyRole,
        AppIconKeyRole,
        AppDesktopRole,
        AppCategoryRole,
        AppGroupRole,
        AppAutoStartRole,
        AppNewInstallRole,
        AppIsOnDesktopRole,
        AppIsOnDockRole,
        AppIsRemovableRole,
        AppIsProxyRole,
        AppEnableScalingRole,
        AppIconSizeRole,
        AppFontSizeRole,
        AppItemIsDraggingRole,
        AppDragIconRole,
        CategoryEnterIconRole,
    };

    enum AppCategory {
        All,
        Custom,
        Search,
        Category,

        // apps category
        Chat,
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
    explicit AppsListModel(const AppCategory& category, QObject *parent = 0);

    inline AppCategory category() const {return m_category;}
    void setDraggingIndex(const QModelIndex &index);
    void setDragDropIndex(const QModelIndex &index);
    void dropInsert(const QString &appKey, const int pos);
    void dropSwap(const int nextPos);

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    const QModelIndex indexAt(const QString &appKey) const;

public slots:
    void clearDraggingIndex();
    void setCategory(const AppCategory category);

protected:
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void dataChanged(const AppsListModel::AppCategory category);
    void layoutChanged(const AppsListModel::AppCategory category);
    bool indexDragging(const QModelIndex &index) const;
//    bool itemIsRemovable(const QString &desktop) const;

private:
    AppsManager *m_appsManager;
    CalculateUtil *m_calcUtil;

    QStringList m_holdPackages;

    QModelIndex m_dragStartIndex = QModelIndex();
    QModelIndex m_dragDropIndex = QModelIndex();
    AppCategory m_category = All;
};

Q_DECLARE_METATYPE(AppsListModel::AppCategory)

#endif // APPSLISTMODEL_H
