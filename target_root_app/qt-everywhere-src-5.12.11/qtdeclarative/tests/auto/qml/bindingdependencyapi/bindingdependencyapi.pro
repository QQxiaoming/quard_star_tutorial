CONFIG += testcase
TARGET = tst_bindingdependencyapi
macos:CONFIG -= app_bundle

SOURCES += tst_bindingdependencyapi.cpp

include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml-private quick-private testlib
