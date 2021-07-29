
cmake_minimum_required(VERSION 2.8)

project(qmake_cmake_files)

enable_testing()

find_package(Qt5Core REQUIRED)

include("${_Qt5CTestMacros}")

test_module_includes(
  Qml QQmlEngine
  Quick QQuickWindow
)

expect_pass(test_plugins)

add_test(qtquickcompiler ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/qtquickcompiler/"
    "${CMAKE_CURRENT_BINARY_DIR}/qtquickcompiler"
    --build-config "${CMAKE_BUILD_TYPE}"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-project qqc_test
    --build-options "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}" ${BUILD_OPTIONS_LIST}
    --test-command qqc_test
)
