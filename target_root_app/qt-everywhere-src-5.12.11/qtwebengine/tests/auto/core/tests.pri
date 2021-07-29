TEMPLATE = app

CONFIG += testcase

VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

SOURCES += $${TARGET}.cpp
INCLUDEPATH += $$PWD

exists($$_PRO_FILE_PWD_/$${TARGET}.qrc): RESOURCES += $${TARGET}.qrc

QT += testlib network webenginewidgets widgets

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"
include(../embed_info_plist.pri)
