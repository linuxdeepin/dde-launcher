/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef GLOBAL
#define GLOBAL

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "controller/dbuscontroller.h"
#include "views/appitemmanager.h"
#include <QString>
#include <QStringList>
#include <libintl.h>
#include <QObject>

#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()
#define appItemManager Singleton<AppItemManager>::instance()

const int NavgationBarLeftMargin = 50;
const int TopBottomGradientHeight = 60;
const QString LauncherServiceName="com.deepin.dde.Launcher";
const QString LauncherPathName="/com/deepin/dde/Launcher";
static uint unInstallConfirmDialogWid=0;

#define CATEGORY_LIST(V) \
    V(Internet)\
    V(Chat)\
    V(Music)\
    V(Video)\
    V(Graphics)\
    V(Game)\
    V(Office)\
    V(Reading)\
    V(Development)\
    V(System)\
    V(Others)

#define ENUM(V) V,
enum class CategoryID {
    All = -1,
    CATEGORY_LIST(ENUM)
};
#undef ENUM

#define STR(V) #V ,
const QStringList CategoryKeys{
    CATEGORY_LIST(STR)
};
#undef STR

inline QString getCategoryNames(QString text){
    char* translatedText = dgettext("application_categories",text.toStdString().c_str());
    QString str{translatedText};
    return str;
}

inline void setUninstallWindowId(uint xWid) {
    unInstallConfirmDialogWid = xWid;
}
inline uint getUninstallWindowId() {
    return unInstallConfirmDialogWid;
}
#endif // GLOBAL
