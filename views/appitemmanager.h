#ifndef APPITEMMANAGER_H
#define APPITEMMANAGER_H

#include <QObject>
#include "fwd.h"
#include "dbusinterface/dbustype.h"


class CategoryItem;
class AppItem;

class AppItemManager : public QObject
{
    Q_OBJECT

public:
    explicit AppItemManager(QObject *parent = 0);
    ~AppItemManager();
    void init();
    void initConnect();
    QMap<QString, ItemInfo>& getItemInfos();
    QList<ItemInfo>& getAppNameItemInfos();
    QList<ItemInfo>& getInstallTimeItemInfos();
    QList<ItemInfo>& getUseFrequencyItemInfos();

    QStringList& getHideKeys();
    CategoryInfoList& getCategoryInfoList();
    CategoryInfoList& getSortedCategoryInfoList();

    QMap<QString, AppItem *> &getAppItems();
    AppItem *getAppItemByKey(const QString& key);

    QMap<QString, CategoryItem *> &getCategoryItems();
    CategoryItem *getCategoryItemByKey(const QString& key);

signals:

public slots:
    void addCategoryItem(const QString& key);
    void addItem(const ItemInfo& itemInfo);
    void addItems(const QList<ItemInfo>& itemInfos);
    void setItemInfos(const QMap<QString, ItemInfo>& infos);
    void setAppNameInfoList(const QList<ItemInfo>& infos);
    void setInstallTimeInfoList(const QList<ItemInfo>& infos);
    void setUseFrequencyInfoList(const QList<ItemInfo>& infos);
    void setCategoryInfoList(const CategoryInfoList& categoryInfoList);

private:
    QMap<QString, ItemInfo> m_itemInfos;
    QList<ItemInfo> m_appNameItemInfoList;
    QList<ItemInfo> m_installtimeItemInfoList;
    QList<ItemInfo> m_useFrequencyItemInfoList;

    QStringList m_hideKeys;
    CategoryInfoList m_categoryInfoList;
    CategoryInfoList m_sortedCategoryInfoList;
    QMap<QString, CategoryItem*> m_categoryItems;
    QMap<QString, AppItem*> m_appItems;
};

#endif // APPITEMMANAGER_H
