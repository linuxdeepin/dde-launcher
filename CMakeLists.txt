cmake_minimum_required(VERSION 3.7)

set(VERSION 4.0)

project(dde-launcher)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

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

# Sources files
file(GLOB_RECURSE SRCS "src/*.h" "src/*.cpp")

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# dev
file(GLOB INTERFACES "src/dbusinterface/*.h")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(Qt5Svg REQUIRED)

pkg_check_modules(XCB_EWMH REQUIRED xcb-ewmh x11 xcursor)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

add_executable(${BIN_NAME} ${SRCS} ${INTERFACES} src/skin.qrc src/widgets/images.qrc)
target_include_directories(${BIN_NAME} PUBLIC ${DtkWidget_INCLUDE_DIRS}
                                              ${XCB_EWMH_INCLUDE_DIRS}
                                              ${DFrameworkDBus_INCLUDE_DIRS}
                                              ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                                              ${QGSettings_INCLUDE_DIRS}
                                              ${PROJECT_BINARY_DIR})

target_link_libraries(${BIN_NAME} PRIVATE
    ${XCB_EWMH_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
)

## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/dde-launcher/translations)

## dev files
install (FILES ${INTERFACES} DESTINATION include/dde-launcher)

## services files
install(FILES src/dbusservices/com.deepin.dde.Launcher.service DESTINATION /usr/share/dbus-1/services)

## icon
install(FILES data/deepin-launcher.svg DESTINATION /usr/share/icons/hicolor/scalable/apps)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
