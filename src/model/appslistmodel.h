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
class ItemInfo_v1;
class QGSettings;
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
        AppListIconRole,
        AppKeyRole,
        AppDesktopRole,
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
        ItemIsDirRole,
        DirItemInfoRole,
        DirAppIconsRole,
        DirNameRole,
        AppDragIconRole,
        AppItemTitleRole,
        AppItemStatusRole,
        ItemRatio,
        DrawBackgroundRole,
        AppHideOpenRole,
        AppHideSendToDesktopRole,
        AppHideSendToDockRole,
        AppHideStartUpRole,
        AppHideUninstallRole,
        AppHideUseProxyRole,
        AppCanOpenRole,
        AppCanSendToDesktopRole,
        AppCanSendToDockRole,
        AppCanStartUpRole,
        AppCanOpenProxyRole,
    };

    enum AppCategory {
        All,
        TitleMode,              // 标题模式
        LetterMode,             // 字母模式
        Search,                 // 本地搜索模式
        Collect,                // 收藏应用
        Dir,                    // 应用抽屉
        PluginSearch,           // 插件搜索
        SearchFilter,           // 搜索前的原始数据模式（sourceModel对应的模式）

        // apps category
        Internet,               // 网络模式
        Chat,                   // 社交模式
        Music,                  // 音乐模式
        Video,                  // 视频模式
        Graphics,               // 图形图像
        Game,                   //
        Office,                 // 办公模式
        Reading,                // 阅读模式
        Development,            // 编程开发模式
        System,                 // 系统管理模式
        Others,
    };

public:
    explicit AppsListModel(const AppCategory& category, QObject *parent = nullptr);
    void setPageIndex(int pageIndex){m_pageIndex = pageIndex;}

    void setDragToDir(bool state);
    bool getDragToDir();

    inline AppCategory category() const {return m_category;}
    void setDraggingIndex(const QModelIndex &index);
    void setDragDropIndex(const QModelIndex &index);
    void dropInsert(const QString &appKey, const int pos);
    void dropSwap(const int nextPos);
    void removeItem();
    inline QModelIndex dragDropIndex() const {return m_dragDropIndex;}

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    const QModelIndex indexAt(const QString &appKey) const;

    void setDrawBackground(bool draw);

    void updateModelData(const QModelIndex dragIndex, const QModelIndex dropIndex);

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
    void itemDataChanged(const ItemInfo_v1 &info);
//    bool itemIsRemovable(const QString &desktop) const;

private:
    AppsManager *m_appsManager;
    QGSettings *m_actionSettings;
    CalculateUtil *m_calcUtil;

    QList<ItemInfo_v1> m_itemList;

    QStringList m_hideOpenPackages;
    QStringList m_hideSendToDesktopPackages;
    QStringList m_hideSendToDockPackages;
    QStringList m_hideStartUpPackages;
    QStringList m_hideUninstallPackages;

    QStringList m_cantOpenPackages;
    QStringList m_cantSendToDesktopPackages;
    QStringList m_cantSendToDockPackages;
    QStringList m_cantStartUpPackages;
    QStringList m_cantUseProxyPackages;
    QStringList m_holdPackages;

    QModelIndex m_dragStartIndex = QModelIndex();
    QModelIndex m_dragDropIndex = QModelIndex();
    AppCategory m_category = All;

    bool m_drawBackground;
    int m_pageIndex;
    bool m_dragToDir;
};
typedef QList<AppsListModel *> PageAppsModelist;

Q_DECLARE_METATYPE(AppsListModel::AppCategory)

#endif // APPSLISTMODEL_H
