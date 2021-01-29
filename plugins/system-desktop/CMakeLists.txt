set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

set(PLUGIN_NAME "system-desktop")

# Sources files
file(GLOB_RECURSE SRCS "*.h" "*.cpp")

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5DBus REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} plugin.json)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ../../src/interfaces/
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${DFrameworkDBus_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-launcher/plugins)
