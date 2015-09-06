#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include "dbusinterface/dbustype.h"


class SignalManager : public QObject
{
    Q_OBJECT
public:
    SignalManager(QObject *parent = 0){
        Q_UNUSED(parent);
    };
    ~SignalManager(){

    };

signals:
    void launcheRefreshed();
    void appNameItemInfoListChanged(const QList<ItemInfo>& infoList);
    void installTimeItemInfoListChanged(const QList<ItemInfo>& infoList);
    void useFrequencyItemInfoListChanged(const QList<ItemInfo>& infoList);
    /*search*/
    void searchItemInfoListChanged(const QList<ItemInfo>& infoList);
    void showSearchResult();
    void startSearched(QString key);
    void search(QString appKey);

    void viewModeChanged(int mode);

    void sortedModeChanged(int mode);
    void categoryModeChanged(int mode);

    void itemInfosChanged(const QMap<QString, ItemInfo>& itemInfos);
    void categoryInfosChanged(const CategoryInfoList& categoryInfoList);

    void hideNavigationButtonByKeys(const QStringList& buttonKeys);

    void viewRowChanged(int row);
    void navigationButtonClicked(QString key);
    void checkNavigationButtonByKey(QString key);

    /*context menu*/
    void contextMenuShowed(QString url, QPoint pos);

    /*mouse release on whole window*/
    void mouseReleased();

    /*unistall delete item in table*/
    void itemDeleted(QString appKey);

    /*Open app by click/Press Enter*/
    void appOpened(QString appKey);
    void appOpenedInCategoryMode();
    void appOpenedInAppMode();

    /*Keyboard event*/
    void keyDirectionPressed(Qt::Key direction);

    /*Hide*/
    void Hide();

    /*show/hide autoStartLabel*/
    void showAutoStartLabel(QString appKey);
    void hideAutoStartLabel(QString appKey);
};

#endif // SIGNALMANAGER_H
