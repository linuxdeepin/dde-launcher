/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DEFINE_H
#define DEFINE_H

#include "logmanager.h"
#include "daemon.h"
#include "widgets/singleton.h"
#include "global.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>


#undef signals
extern "C" {
  #include <gtk/gtk.h>
}
#define signals public

static void requrestUpdateIcons()
{
    GtkIconTheme* gs = gtk_icon_theme_get_default();
    auto a = gtk_icon_theme_get_example_icon_name(gs);
    if (a != NULL) g_free(a);
    //can not passing QObject to the callback function,so use signal
    emit signalManager->gtkIconThemeChanged();
}

void initGtkThemeWatcher()
{
    GtkIconTheme* gs = gtk_icon_theme_get_default();
    g_signal_connect(gs, "changed",
                     G_CALLBACK(requrestUpdateIcons), NULL);
    auto a = gtk_icon_theme_get_example_icon_name(gs);
    if (a != NULL) g_free(a);
}

void debug_daemon_off(){
    #if defined(QT_NO_DEBUG)
    daemonize();
    #endif
}


// let startdde know that we've already started.
void RegisterDdeSession()
{
    const QString envName = "DDE_SESSION_PROCESS_COOKIE_ID";
    QByteArray cookie = qgetenv(envName.toLatin1().constData());
    qunsetenv(envName.toLatin1().constData());

    if (!cookie.isEmpty()) {
        QDBusInterface iface("com.deepin.SessionManager",
                             "/com/deepin/SessionManager",
                             "com.deepin.SessionManager",
                             QDBusConnection::sessionBus());
        iface.asyncCall("Register", QString(cookie));
    }
}


void debug_log_console_on(){
    #if !defined(QT_NO_DEBUG)
    LogManager::instance()->initConsoleAppender();
    #endif
    LogManager::instance()->initRollingFileAppender();
}

#endif // DEFINE_H

