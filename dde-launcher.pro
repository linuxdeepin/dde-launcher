QT      += core gui dbus widgets x11extras svg

TARGET = dde-launcher
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkbase dtkwidget dtkutil xcb xcb-ewmh\
          gsettings-qt gtk+-2.0 gio-unix-2.0


include(./widgets/widgets.pri)
include(./background/background.pri)
include(./dbusinterface/dbusinterface.pri)

SOURCES += \
    mainframe.cpp \
    model/appslistmodel.cpp \
    model/appsmanager.cpp \
    view/applistview.cpp \
    global_util/util.cpp \
    delegate/appitemdelegate.cpp \
    global_util/xcb_misc.cpp \
    worker/menuworker.cpp \
    dbusservices/dbuslauncherservice.cpp \
    main.cpp \
    global_util/calculate_util.cpp \
    global_util/themeappicon.cpp

HEADERS += \
    mainframe.h \
    model/appslistmodel.h \
    model/appsmanager.h \
    view/applistview.h \
    global_util/constants.h \
    global_util/util.h \
    delegate/appitemdelegate.h \
    global_util/xcb_misc.h \
    worker/menuworker.h \
    dbusservices/dbuslauncherservice.h \
    global_util/calculate_util.h \
    global_util/themeappicon.h

#Automating generation .qm files from .ts files
system($$PWD/translate_generation.sh)

target.path = /usr/bin

qm_files.files = translations/*.qm
qm_files.path = /usr/share/dde-launcher/translations

services.path = /usr/share/dbus-1/services
services.files = dbusservices/com.deepin.dde.Launcher.service

INSTALLS += target qm_files services

RESOURCES += \
    skin.qrc
