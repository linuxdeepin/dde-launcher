#ifndef DBUSCONTROLLER_H
#define DBUSCONTROLLER_H


#include <QObject>
#include "dbusinterface/dbustype.h"

class LauncherInterface;
class FileInfoInterface;

#define Launcher_service "com.deepin.dde.daemon.Launcher"
#define Launcher_path "/com/deepin/dde/daemon/Launcher"

#define FileInfo_service "com.deepin.filemanager.Backend.FileInfo"
#define FileInfo_path "/com/deepin/filemanager/Backend/FileInfo"

class DBusController : public QObject
{
    Q_OBJECT
public:
    explicit DBusController(QObject *parent = 0);
    ~DBusController();
    void init();
    void getCategoryInfoList();
    void getInstalledTimeItems();
    void getAllFrequencyItems();
    ItemInfo getItemInfo(QString appKey);
    QString getThemeIconByAppKey(QString appKey, int size=48);

signals:

public slots:

private:
    LauncherInterface* m_launcherInterface;
    FileInfoInterface* m_fileInfoInterface;
    AppInstalledTimeInfoList m_appInstalledTimeInfoList;
    AppFrequencyInfoList m_appFrequencyInfoList;
    CategoryInfoList m_categoryInfoList;
    QMap<QString, QString> m_appIcons;
    QMap<QString, ItemInfo> m_itemInfos;
};

#endif // DBUSCONTROLLER_H
