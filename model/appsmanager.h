#ifndef APPSMANAGER_H
#define APPSMANAGER_H

#include "appslistmodel.h"
#include "dbuslauncher.h"
#include "dbusfileinfo.h"
#include "dbustartmanager.h"
#include "dbusdockedappmanager.h"
#include "dbusdisplay.h"
#include "global_util/calculate_util.h"

#include <QMap>
#include <QSettings>
#include <QPixmap>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDBusArgument>
#include <QList>

class AppsManager : public QObject
{
    Q_OBJECT

public:
    static AppsManager *instance(QObject *parent = nullptr);
    CalculateUtil* calUtil;
signals:

    void dataChanged(const AppsListModel::AppCategory category) const;
    void handleUninstallApp(const QModelIndex &index, int result);
    void updateCategoryView(const AppsListModel::AppCategory categoryInfo) const;
    void primaryChanged();
public slots:
    void searchApp(const QString &keywords);
    void launchApp(const QModelIndex &index);
    const ItemInfoList appsInfoList(const AppsListModel::AppCategory &category) const;

    bool appIsNewInstall(const QString &key);
    bool appIsAutoStart(const QString &desktop);
    bool appIsOnDock(const QString &appName);
    bool appIsOnDesktop(const QString &desktop);
    const QPixmap appIcon(const QString &desktop);
    int appNums(const AppsListModel::AppCategory &category) const;

    //remove the item icon firstly, when unInstalling apps
    void unInstallApp(const QModelIndex &index, int value);
    //restore the itemInfo, if unInstall failed!
    void reStoreItem();

    QRect getPrimayRect();

private:
    explicit AppsManager(QObject *parent = 0);

    void appendSearchResult(const QString &appKey);
    void sortCategory(const AppsListModel::AppCategory category);
    void sortByName(ItemInfoList &processList);
    void refreshCategoryInfoList();
    void refreshAppIconCache();
    void refreshAppAutoStartCache();
    const QPixmap loadSvg(const QString &fileName, const int size);

private slots:
    void searchDone(const QStringList &resultList);

private:
    DBusLauncher *m_launcherInter;
    DBusFileInfo *m_fileInfoInter;
    DBusStartManager *m_startManagerInter;
    DBusDockedAppManager *m_dockedAppInter;
    DBusDisplay* m_displayInterface;

    QTimer *m_searchTimer;

    QString m_searchText;
    QStringList m_newInstalledAppsList;
    ItemInfoList m_appInfoList;
    ItemInfoList m_appSearchResultList;
    QMap<AppsListModel::AppCategory, ItemInfoList> m_appInfos;

    static AppsManager *INSTANCE;
    static QSettings APP_ICON_CACHE;
    static QSettings APP_AUTOSTART_CACHE;
    ItemInfo m_unInstallItem = ItemInfo();
};

#endif // APPSMANAGER_H
