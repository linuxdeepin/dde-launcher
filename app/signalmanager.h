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
    void requestData();
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
    void rightClickedChanged(bool flag);

    /*mouse release on whole window*/
    void mouseReleased();
    void rightMouseReleased(QString url);

    /*checked item or not*/
    void highlightChanged(bool isHighLight);

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

    /*check first category button when hide*/
    void firstButtonChecked();

    /*screen geometry changed*/
    void screenGeometryChanged();

    /*handle unistall action*/
    void appUninstalled(QString appKey);
    void uninstallActionChanged(QString appKey, int id);

    /*active changed signal from dock dbus client*/
    void activeWindowChanged(uint windowId);

    /*installTime refresh*/
    void installTimeRefreshed();

    /*frequency refresh*/
    void frequencyRefreshed();

    /*AppItem dragging state changed*/
    void appItemDragStateChanged(bool flag);

    /*hide refresh installtime or frequency data*/
    void refreshInstallTimeFrequency();

    /*gtk theme changed*/
    void gtkIconThemeChanged();

    /*hide newinstalllindicator*/
    void newinstalllindicatorHided(QString appKey);
    void newinstalllindicatorShowed(QString appKey);
};

#endif // SIGNALMANAGER_H
