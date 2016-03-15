QT      += core gui dbus widgets x11extras

TARGET = dde-launcher
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
<<<<<<< 16a3f3c2de91333e65d6f2a07c226cd9aa1f94e1
<<<<<<< 54ba9a743079cb32f72588bef99b663ccf0ea760
<<<<<<< 90f7166dcf66c2ab57efac31c6fec82a4ab5147f
<<<<<<< 7a39e0d3c3ca3ab83dd188d5e7018fbd62688ac9
<<<<<<< 1c301c7a53dd948b5c5fa1c81718826a55b6d514

DEFINES += QT_MESSAGELOGCONTEXT

include(./widgets/widgets.pri)
include(./cutelogger/cutelogger.pri)
include(./dialogs/dialogs.pri)
include(./background/background.pri)

SOURCES += main.cpp \
    views/launcherframe.cpp \
    views/navigationbar.cpp \
    views/basetablewidget.cpp \
    views/apptablewidget.cpp \
    views/categoryframe.cpp \
    app/xcb_misc.cpp \
    app/daemon.cpp \
    app/launcherapp.cpp \
    views/categorytablewidget.cpp \
    views/displaymodeframe.cpp \
    views/basecheckedbutton.cpp \
    views/baseframe.cpp \
    views/viewmodebutton.cpp \
    views/navigationbuttonframe.cpp \
    dbusinterface/launcher_interface.cpp \
    dbusinterface/dbustype.cpp \
    controller/dbuscontroller.cpp \
    dbusinterface/fileInfo_interface.cpp \
    views/appitem.cpp \
    views/categoryitem.cpp \
    dbusinterface/menumanager_interface.cpp \
    dbusinterface/menu_interface.cpp \
    controller/menucontroller.cpp \
    dbusinterface/dbusdockedappmanager.cpp \
    views/borderbutton.cpp \
    dbusinterface/startmanager_interface.cpp \
    dbusinterface/launcheradaptor.cpp \
    dbusinterface/dde_launchder_interface.cpp \
    views/searchlineedit.cpp \
    dbusinterface/launchersettings_interface.cpp \
    views/dlineedit.cpp \
    controller/dbusworker.cpp \
    dbusinterface/notification_interface.cpp \
    dbusinterface/displayinterface.cpp \
    views/gradientlabel.cpp \
    dbusinterface/dbusclientmanager.cpp \
    dbusinterface/pinyin_interface.cpp \
    dbusinterface/dock_interface.cpp \
    views/appitemmanager.cpp

HEADERS += \
    views/launcherframe.h \
    views/navigationbar.h \
    views/basetablewidget.h \
    views/apptablewidget.h \
    views/categoryframe.h \
    app/define.h \
    app/xcb_misc.h \
    app/daemon.h \
    app/launcherapp.h \
    views/categorytablewidget.h \
    app/signalmanager.h \
    app/global.h \
    views/displaymodeframe.h \
    views/basecheckedbutton.h \
    views/baseframe.h \
    views/viewmodebutton.h \
    views/navigationbuttonframe.h \
    dbusinterface/launcher_interface.h \
    dbusinterface/dbustype.h \
    controller/dbuscontroller.h \
    dbusinterface/fileInfo_interface.h \
    views/appitem.h \
    views/fwd.h \
    views/categoryitem.h \
    dbusinterface/menumanager_interface.h \
    dbusinterface/menu_interface.h \
    controller/menucontroller.h \
    dbusinterface/dbusdockedappmanager.h \
    views/borderbutton.h \
    dbusinterface/startmanager_interface.h \
    dbusinterface/launcheradaptor.h \
    dbusinterface/dde_launcher_interface.h \
    views/searchlineedit.h \
    dbusinterface/launchersettings_interface.h \
    views/dlineedit.h \
    controller/dbusworker.h \
    dbusinterface/notification_interface.h \
    dbusinterface/displayinterface.h \
    views/gradientlabel.h \
    dbusinterface/dbusclientmanager.h \
    dbusinterface/pinyin_interface.h \
    dbusinterface/dock_interface.h \
    views/appitemmanager.h

# Automating generation .qm files from .ts files
system($$PWD/translate_generation.sh)

RESOURCES += \
    skin.qrc \
    dialogs.qrc

target.path = /usr/bin/

qm_files.files = translations/*.qm
qm_files.path  = /usr/share/dde-launcher/translations

services.path = /usr/share/dbus-1/services
services.files = dbusservices/com.deepin.dde.Launcher.service

INSTALLS += target qm_files services
=======
PKGCONFIG += dtkbase dtkwidget
=======
PKGCONFIG += dtkbase dtkwidget dtkbase
>>>>>>> test widgets list auto-resize
=======
PKGCONFIG += dtkbase dtkwidget dtkbase gtk+-2.0 gio-unix-2.0

include(./widgets/widgets.pri)
>>>>>>> adjust ui
=======
PKGCONFIG += dtkbase dtkwidget dtkbase
>>>>>>> Change main frame layout
=======
PKGCONFIG += dtkbase dtkwidget dtkbase xcb xcb-ewmh
>>>>>>> Add StartManager dbus interface

SOURCES += \
    mian.cpp \
    mainframe.cpp \
    dbus/dbuslauncher.cpp \
    model/appslistmodel.cpp \
    model/appsmanager.cpp \
    view/applistview.cpp \
    widget/categorytitlewidget.cpp \
    widget/searchwidget.cpp \
    global_util/util.cpp \
    dbus/dbusfileinfo.cpp \
    dbus/dbusvariant/categoryinfo.cpp \
    dbus/dbusvariant/frequencyinfo.cpp \
    dbus/dbusvariant/installedtimeinfo.cpp \
    dbus/dbusvariant/iteminfo.cpp \
    delegate/appitemdelegate.cpp \
    dbus/dbustartmanager.cpp \
    global_util/xcb_misc.cpp \
    widget/navigationwidget.cpp \
    widget/categorybutton.cpp \
    dbus/dbusmenu.cpp \
    dbus/dbusmenumanager.cpp \
    worker/menuworker.cpp \
    dbus/dbusdockedappmanager.cpp

HEADERS += \
    mainframe.h \
    dbus/dbuslauncher.h \
    model/appslistmodel.h \
<<<<<<< 8d5b28b6906b7624eebe3dacb3297fa0bca63884
    model/appsmanager.h
>>>>>>> write new launcher, remove all old files
=======
    model/appsmanager.h \
<<<<<<< 90f7166dcf66c2ab57efac31c6fec82a4ab5147f
<<<<<<< 98e7ec7e4e49fa874938262caabaad1f85e8c038
    appitemdelegate.h
>>>>>>> add item delegate to draw cell
=======
    appitemdelegate.h \
    view/applistview.h \
<<<<<<< 34aabf291ea7539f96db0b0a9c2cd0864740c06b
    widget/categorytitlewidget.h
>>>>>>> adjust directory structure
=======
    widget/categorytitlewidget.h \
    widget/searchwidget.h \
=======
    view/applistview.h \
<<<<<<< 54ba9a743079cb32f72588bef99b663ccf0ea760
>>>>>>> adjust ui
=======
    widget/categorytitlewidget.h \
    widget/searchwidget.h \
>>>>>>> Change main frame layout
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
    widget/navigationwidget.h \
    widget/categorybutton.h \
    dbus/dbusmenu.h \
    dbus/dbusmenumanager.h \
    worker/menuworker.h \
    dbus/dbusdockedappmanager.h

target.path = /usr/bin

qm_files.files = translations/*.qm
qm_files.path = /usr/share/dde-launcher/translations

service.path = /usr/share/dbus-1/services
service.files = dbusservices/com.deepin.dde.Launcher.service

INSTALLS += target qm_files services
<<<<<<< 41d93f3d414f7c9f67c107a1883ab90353669dfd
>>>>>>> add dnd support
=======

RESOURCES += \
    skin.qrc
>>>>>>> add the left navigationbar
