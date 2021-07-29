
cmake_minimum_required(VERSION 2.8)
project(qqc_test)

find_package(Qt5Qml 5.0.0 REQUIRED)
find_package(Qt5Gui 5.0.0 REQUIRED)
find_package(Qt5Test 5.0.0 REQUIRED)
find_package(Qt5QuickCompiler)

set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

qtquick_compiler_add_resources(RESOURCES "empty.qrc" "resources with space/qqc_test.qrc")

set(CMAKE_AUTOMOC ON)

add_executable(qqc_test "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" ${RESOURCES})
target_link_libraries(qqc_test Qt5::Gui Qt5::Qml Qt5::Test)
