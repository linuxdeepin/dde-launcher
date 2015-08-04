#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include "dbusinterface/dbustype.h"


class SignalManager : public QObject
{
    Q_OBJECT
public:
    SignalManager(QObject *parent = 0){

    };
    ~SignalManager(){

    };

signals:
    void categoryModeChanged(int mode);
    void sortModeChanged(int mode);
    void itemInfosChanged(const QMap<QString, ItemInfo>& itemInfos);
    void categoryInfosChanged(const CategoryInfoList& categoryInfoList);

    void hideNavigationButtonByKeys(const QStringList& buttonKeys);

    void viewRowChanged(int row);
    void navigationButtonClicked(QString key);
    void checkNavigationButtonByKey(QString key);

public slots:
};

#endif // SIGNALMANAGER_H
