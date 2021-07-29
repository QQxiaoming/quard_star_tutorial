CONFIG += testcase
TARGET = tst_customization
SOURCES += tst_customization.cpp

macos:CONFIG -= app_bundle

QT += core-private gui-private qml-private quick-private testlib quicktemplates2-private quickcontrols2

include (../shared/util.pri)

TESTDATA = data/*
