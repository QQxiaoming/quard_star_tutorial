TEMPLATE = app
TARGET = tst_creationtime

QT += qml testlib
CONFIG += testcase
macos:CONFIG -= app_bundle

include(../../auto/shared/util.pri)

SOURCES += \
    tst_creationtime.cpp
