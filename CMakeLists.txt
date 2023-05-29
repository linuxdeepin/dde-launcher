# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)

set(VERSION 4.0)

project(dde-launcher)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -z relro -z now -z noexecstack -pie")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

if (DEFINED DISABLE_DRAG_ANIMATION)
    add_definitions(-DDISABLE_DRAG_ANIMATION)
endif ()

if (DEFINED WITHOUT_UNINSTALL_APP)
    add_definitions(-DWITHOUT_UNINSTALL_APP)
endif ()

set(BIN_NAME dde-launcher)

#add_subdirectory("tests")
add_subdirectory("plugins")

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
include(GNUInstallDirs)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED
    COMPONENTS Widgets Svg X11Extras DBus Concurrent
)
find_package(Qt5Gui CONFIG REQUIRED Private)
find_package(Dtk REQUIRED COMPONENTS Core Widget Gui)

pkg_check_modules(XCB_EWMH REQUIRED IMPORTED_TARGET xcb-ewmh)
pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)

set(XML2CPP_DIR ${PROJECT_SOURCE_DIR}/src/global_util/dbus/xml2cpp)

function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND qdbusxml2cpp-fix -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${XML2CPP_DIR})
endfunction(generation_dbus_interface)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.ImageEffect1.xml
    ImageEffect
    ${XML2CPP_DIR}/imageeffect_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.ImageBlur1.xml
    ImageBlur
    ${XML2CPP_DIR}/imageblur_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/com.deepin.wm.xml
    wm
    ${XML2CPP_DIR}/wm_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Display1.xml
    Display
    ${XML2CPP_DIR}/display_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Appearance1.xml
    Appearance
    ${XML2CPP_DIR}/appearance_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Gesture1.xml
    Gesture
    ${XML2CPP_DIR}/gesture_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Accounts1.User.xml
    AccountsUser
    ${XML2CPP_DIR}/accountsuser_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.Accounts1.xml
    Accounts
    ${XML2CPP_DIR}/accounts_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.SessionManager1.xml
    SessionManager
    ${XML2CPP_DIR}/sessionmanager_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.XEventMonitor1.xml
    XEventMonitor
    ${XML2CPP_DIR}/xeventmonitor_interface
)

add_library(trashutils
OBJECT
    src/trashutils/trashmonitor.h
    src/trashutils/trashmonitor.cpp
)

target_link_libraries(trashutils
PUBLIC
    Qt5::Core
    PkgConfig::GIO
)

target_compile_definitions(trashutils
PRIVATE
    QT_NO_KEYWORDS
)

include_directories(
    src
    src/boxframe
    src/dbusinterface
    src/dbusinterface/dbusvariant
    src/dbusservices
    src/delegate
    src/global_util
    src/global_util/dbus/qtdbusextended
    src/global_util/dbus/xml2cpp
    src/global_util/dbus/xml2cpp/types
    src/model
    src/skin
    src/view
    src/widgets
    src/worker
    interfaces
)

aux_source_directory(src SRC)
aux_source_directory(src/boxframe BOXFRAME)
aux_source_directory(src/dbusinterface DBUSINTERFACE)
aux_source_directory(src/dbusinterface/dbusvariant DBUSVARIANT)
aux_source_directory(src/dbusservices DBUSSERVICES)
aux_source_directory(src/delegate DELEGATE)
aux_source_directory(src/global_util GLOBAL_UTIL)
aux_source_directory(src/model MODEL)
aux_source_directory(src/skin SKIN)
aux_source_directory(src/view VIEW)
aux_source_directory(src/widgets WIDGETS)
aux_source_directory(src/worker WORKER)
aux_source_directory(src/global_util/dbus/xml2cpp/types DBUS_TYPES)
aux_source_directory(src/global_util/dbus/xml2cpp/ DBUS_INTERFACES)

file(GLOB PLUGININTERFACES "interfaces/*.h")
file(GLOB PLUGINCONTROLLER "controller/*.*")
file(GLOB SRC_PATH
    ${SRC}
    ${BOXFRAME}
    ${DBUSINTERFACE}
    ${DBUSVARIANT}
    ${DBUSSERVICES}
    ${DELEGATE}
    ${GLOBAL_UTIL}
    ${MODEL}
    ${SKIN}
    ${VIEW}
    ${WIDGETS}
    ${WORKER}
    ${PLUGININTERFACES}
    ${PLUGINCONTROLLER}
    ${DBUS_TYPES}
    ${DBUS_INTERFACES}
)

add_executable(${BIN_NAME} ${SRC_PATH} ${INTERFACES} src/skin.qrc src/widgets/images.qrc)

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
    PkgConfig::XCB_EWMH
    Dtk::Core
    ${DtkWidget_LIBRARIES} # seems we won't be able to use Dtk::Widget in dtkwidget 5.6.4
    Dtk::Gui
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::X11Extras
    Qt5::DBus
    Qt5::Svg
    Qt5::GuiPrivate
    PkgConfig::QGSettings
    trashutils
)

## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-launcher/translations)

## desktop file
install(FILES dde-launcher.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/)

configure_file(${CMAKE_SOURCE_DIR}/dde-launcher-wapper.in ${CMAKE_CURRENT_BINARY_DIR}/dde-launcher-wapper @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-launcher-wapper DESTINATION ${CMAKE_INSTALL_BINDIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

## services files
configure_file(
    ${CMAKE_SOURCE_DIR}/src/dbusservices/org.deepin.dde.Launcher1.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.Launcher1.service
    @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.Launcher1.service 
	DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

#schemas
install(FILES gschema/com.deepin.dde.launcher.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)
install(CODE "execute_process(COMMAND glib-compile-schemas ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)")

## icon
install(FILES data/deepin-launcher.svg DESTINATION share/icons/hicolor/scalable/apps)

# bin
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

#dev
file(GLOB INTERFACE_FILES "interfaces/*.h")
install(FILES ${INTERFACE_FILES} DESTINATION include/dde-launcher)

# config
file(GLOB DCONFIG_FILES "configs/*.json")
install(FILES ${DCONFIG_FILES} DESTINATION share/dsg/configs/dde-launcher)
dconfig_meta_files(APPID org.deepin.dde.launcher BASE ./config/*.json)
