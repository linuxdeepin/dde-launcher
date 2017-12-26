QT      += core gui dbus widgets x11extras svg

TARGET = dde-launcher
TEMPLATE = app
CONFIG += c++14 link_pkgconfig
PKGCONFIG += dtkwidget xcb xcb-ewmh \
          gsettings-qt dframeworkdbus

include(./feature-macros.pri)
include(./widgets/widgets.pri)
include(./boxframe/boxframe.pri)
include(./dbusinterface/dbusinterface.pri)

ARCH = $$QMAKE_HOST.arch
isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    DEFINES += ARCH_MIPSEL
}

isEqual(LAUNCHER_AUTO_EXIT, YES) {
    DEFINES += LAUNCHER_AUTO_EXIT
}

PKGCONFIG += dtkcore

SOURCES += \
    model/appslistmodel.cpp \
    model/appsmanager.cpp \
    global_util/util.cpp \
    delegate/appitemdelegate.cpp \
    global_util/xcb_misc.cpp \
    worker/menuworker.cpp \
    dbusservices/dbuslauncherservice.cpp \
    main.cpp \
    global_util/calculate_util.cpp \
    launchersys.cpp \
    fullscreenframe.cpp \
    miniframe.cpp \
    view/appgridview.cpp \
    view/applistview.cpp \
    delegate/applistdelegate.cpp \
    sharedeventfilter.cpp \
    global_util/recentlyused.cpp \
    historywidget.cpp \
    model/historymodel.cpp

HEADERS += \
    model/appslistmodel.h \
    model/appsmanager.h \
    global_util/constants.h \
    global_util/util.h \
    delegate/appitemdelegate.h \
    global_util/xcb_misc.h \
    worker/menuworker.h \
    dbusservices/dbuslauncherservice.h \
    global_util/calculate_util.h \
    launchersys.h \
    launcherinterface.h \
    fullscreenframe.h \
    miniframe.h \
    view/appgridview.h \
    view/applistview.h \
    delegate/applistdelegate.h \
    sharedeventfilter.h \
    global_util/recentlyused.h \
    historywidget.h \
    model/historymodel.h

#Automating generation .qm files from .ts files
system($$PWD/translate_generation.sh)

target.path = /usr/bin

data_files.files = data/*.json data/*.conf
data_files.path = /usr/share/dde-launcher/data

qm_files.files = translations/*.qm
qm_files.path = /usr/share/dde-launcher/translations

services.path = /usr/share/dbus-1/services
services.files = dbusservices/com.deepin.dde.Launcher.service

icons.path = /usr/share/icons/hicolor/scalable/apps
icons.files = data/*.svg

INSTALLS += target qm_files services data_files icons

RESOURCES += \
    skin.qrc

DISTFILES += data/*
