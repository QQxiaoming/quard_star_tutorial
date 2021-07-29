include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore-private

TEMPLATE = app

CONFIG += testcase
CONFIG += c++14

VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

SOURCES += $${TARGET}.cpp
INCLUDEPATH += $$PWD

exists($$_PRO_FILE_PWD_/$${TARGET}.qrc): RESOURCES += $${TARGET}.qrc

QT += testlib network webenginewidgets widgets quick quickwidgets

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"

include(../embed_info_plist.pri)
