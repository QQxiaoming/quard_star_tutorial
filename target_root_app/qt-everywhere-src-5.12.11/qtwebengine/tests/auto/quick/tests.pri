include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore-private

TEMPLATE = app

CONFIG += testcase

VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

SOURCES += $${TARGET}.cpp
INCLUDEPATH += \
    $$PWD \
    ../shared

QT += testlib network quick webengine

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"
include(../embed_info_plist.pri)
