#! [0]
cmake_minimum_required(VERSION 3.1.0)

project(helloworld)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(helloworld
    mainwindow.ui
    mainwindow.cpp
    main.cpp
    resources.qrc
)

target_link_libraries(helloworld Qt5::Widgets)
#! [0]

#! [1]
find_package(Qt5 COMPONENTS Core REQUIRED)

get_target_property(QtCore_location Qt5::Core LOCATION)
#! [1]

#! [2]
find_package(Qt5 COMPONENTS Core REQUIRED)

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")

# set up a mapping so that the Release configuration for the Qt imported target is
# used in the COVERAGE CMake configuration.
set_target_properties(Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE")
#! [2]

#! [5]
foreach(plugin ${Qt5Network_PLUGINS})
  get_target_property(_loc ${plugin} LOCATION)
  message("Plugin ${plugin} is at location ${_loc}")
endforeach()
#! [5]

