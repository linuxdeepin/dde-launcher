QT      += core gui dbus widgets x11extras svg

TARGET = dde-launcher
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkbase dtkwidget dtkbase xcb xcb-ewmh


include(./widget/widget.pri)

SOURCES += \
    mian.cpp \
    mainframe.cpp \
    dbus/dbuslauncher.cpp \
    model/appslistmodel.cpp \
    model/appsmanager.cpp \
    view/applistview.cpp \
    global_util/util.cpp \
    dbus/dbusfileinfo.cpp \
    dbus/dbusvariant/categoryinfo.cpp \
    dbus/dbusvariant/frequencyinfo.cpp \
    dbus/dbusvariant/installedtimeinfo.cpp \
    dbus/dbusvariant/iteminfo.cpp \
    delegate/appitemdelegate.cpp \
    dbus/dbustartmanager.cpp \
    global_util/xcb_misc.cpp \
    dbus/dbusmenu.cpp \
    dbus/dbusmenumanager.cpp \
    worker/menuworker.cpp \
    dbus/dbusdockedappmanager.cpp \
    dbusservices/dbuslauncherservice.cpp \
    dbus/dbuslauncherframe.cpp

HEADERS += \
    mainframe.h \
    dbus/dbuslauncher.h \
    model/appslistmodel.h \
    model/appsmanager.h \
    view/applistview.h \
    global_util/constants.h \
    global_util/util.h \
    dbus/dbusfileinfo.h \
    dbus/dbusvariant/categoryinfo.h \
    dbus/dbusvariant/frequencyinfo.h \
    dbus/dbusvariant/installedtimeinfo.h \
    dbus/dbusvariant/iteminfo.h \
    delegate/appitemdelegate.h \
    dbus/dbustartmanager.h \
    global_util/xcb_misc.h \
    dbus/dbusmenu.h \
    dbus/dbusmenumanager.h \
    worker/menuworker.h \
    dbus/dbusdockedappmanager.h \
    dbusservices/dbuslauncherservice.h \
    dbus/dbuslauncherframe.h

target.path = /usr/bin

qm_files.files = translations/*.qm
qm_files.path = /usr/share/dde-launcher/translations

services.path = /usr/share/dbus-1/services
services.files = dbusservices/com.deepin.dde.Launcher.service

INSTALLS += target qm_files services

RESOURCES += \
    skin.qrc
