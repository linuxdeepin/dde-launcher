#ifndef APPITEMMANAGER_H
#define APPITEMMANAGER_H

#include <QObject>
#include "fwd.h"

class ItemInfo;
//class AppItemPointer;

class AppItemManager : public QObject
{
    Q_OBJECT
public:
    explicit AppItemManager(QObject *parent = 0);
    ~AppItemManager();

    void init();
    void initConnect();

    QMap<QString, AppItemPointer> getAppItems();
    QList<AppItemPointer> getAppItemsByNameSorted();
    QList<AppItemPointer> getAppItemsByInstalledTime();
    QList<AppItemPointer> getAppItemsByUseFrequency();
signals:

public slots:
    void createAppItem(const ItemInfo& itemInfo, int index);
    void createAppItems(const QMap<QString, ItemInfo>& itemInfos);

private:
    QMap<QString, AppItemPointer> m_appItems;
};

#endif // APPITEMMANAGER_H
