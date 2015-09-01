#ifndef DBUSCONTROLLER_H
#define DBUSCONTROLLER_H


#include <QObject>
#include "dbusinterface/dbustype.h"

class LauncherInterface;
class LauncherSettingsInterface;
class FileInfoInterface;
class StartManagerInterface;
class MenuController;

#define Launcher_service "com.deepin.dde.daemon.Launcher"
#define Launcher_path "/com/deepin/dde/daemon/Launcher"

#define FileInfo_service "com.deepin.filemanager.Backend.FileInfo"
#define FileInfo_path "/com/deepin/filemanager/Backend/FileInfo"

#define StartManager_service "com.deepin.SessionManager"
#define StartManager_path "/com/deepin/StartManager"

bool appNameLessThan(const ItemInfo &info1, const ItemInfo &info2);
bool installTimeLessThan(const ItemInfo &info1, const ItemInfo &info2);
bool useFrequencyLessThan(const AppFrequencyInfo &info1, const AppFrequencyInfo &info2);

class DBusController : public QObject
{
    Q_OBJECT
public:
    explicit DBusController(QObject *parent = 0);
    ~DBusController();
    void initConnect();
    void getCategoryInfoList();
    void getInstalledTimeItems();
    void getAllFrequencyItems();
    int getSortMethod();
    int getCategoryDisplayMode();


    LauncherInterface* getLauncherInterface();
    StartManagerInterface* getStartManagerInterface();
    ItemInfo getItemInfo(QString appKey);

    void sortedByAppName(QList<ItemInfo> infos);
    void sortedByInstallTime(QList<ItemInfo> infos);

signals:

public slots:
    void init();
    void updateAppTable(QString appKey);
    void searchDone(QStringList appKeys);
    void setCategoryDisplayMode(int mode);
    void setSortMethod(int mode);

private:
    LauncherInterface* m_launcherInterface;
    LauncherSettingsInterface* m_launcherSettingsInterface;
    FileInfoInterface* m_fileInfoInterface;
    StartManagerInterface* m_startManagerInterface;
    MenuController* m_menuController;
    AppInstalledTimeInfoList m_appInstalledTimeInfoList;
    AppFrequencyInfoList m_appFrequencyInfoList;
    CategoryInfoList m_categoryInfoList;
    CategoryInfoList m_categoryAppNameSortedInfoList;

    QMap<QString, ItemInfo> m_itemInfos;
    QMap<QString, QStringList> m_categoryAppNames;

    QList<ItemInfo> m_appNameSortedList;
    QList<ItemInfo> m_installTimeSortedList;
    QList<ItemInfo> m_useFrequencySortedList;
    QList<ItemInfo> m_searchList;
};

#endif // DBUSCONTROLLER_H
