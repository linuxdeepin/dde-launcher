#ifndef DEFINE_H
#define DEFINE_H

#include "logmanager.h"
#include "daemon.h"
#include "widgets/singleton.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>

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
    Singleton<LogManager>::instance()->initConsoleAppender();
    #endif
    Singleton<LogManager>::instance()->initRollingFileAppender();
}

#endif // DEFINE_H

