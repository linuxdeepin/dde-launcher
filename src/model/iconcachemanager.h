// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONCACHEMANAGER_H
#define ICONCACHEMANAGER_H

#include "iteminfo.h"

#include <QObject>
#include <QString>
#include <QHash>
#include <QPixmap>
#include <QReadWriteLock>

class IconCacheManager : public QObject
{
    Q_OBJECT
public:
    static IconCacheManager *instance();
    static void resetIconData();

    static bool iconLoadState();
    static void setIconLoadState(bool state);

    static bool existInCache(const QPair<QString, int> &tmpKey);
    static void getPixFromCache(QPair<QString, int> &tmpKey, QPixmap &pix);
    static void insertCache(const QPair<QString, int> &tmpKey, const QPixmap &pix);

private:
    explicit IconCacheManager(QObject *parent = nullptr);

    void createPixmap(const ItemInfo &itemInfo, int size);
    void removeItemFromCache(const ItemInfo &info);
    double getCurRatio();

signals:
    void iconLoaded();

public slots:
    void loadWindowIcon();
    void loadOtherIcon();

    void loadCurRatioIcon(int mode);
    void loadOtherRatioIcon(int mode);
    void loadFullWindowIcon();

    void loadItem(const ItemInfo &info, const QString &operationStr);
    void updateCanlendarIcon();

private:
    static QReadWriteLock m_iconLock;
    static QHash<QPair<QString, int>, QPixmap> m_iconCache;
    static std::atomic<bool> m_loadState;

    ItemInfo m_calendarInfo;
    bool m_iconValid;
    int m_tryNums;
    int m_tryCount;
    QDate m_date;
};

#endif // ICONCACHEMANAGER_H
