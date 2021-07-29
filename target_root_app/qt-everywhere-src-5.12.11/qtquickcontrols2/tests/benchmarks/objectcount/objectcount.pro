TEMPLATE = app
TARGET = tst_objectcount

QT += quick testlib core-private
CONFIG += testcase
macos:CONFIG -= app_bundle

include(../../auto/shared/util.pri)

SOURCES += \
    tst_objectcount.cpp
