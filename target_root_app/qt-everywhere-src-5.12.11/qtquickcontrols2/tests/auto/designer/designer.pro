TEMPLATE = app
TARGET = tst_designer

QT += quick quick-private quickcontrols2 testlib
CONFIG += testcase
macos:CONFIG -= app_bundle

SOURCES += \
    $$PWD/tst_designer.cpp
