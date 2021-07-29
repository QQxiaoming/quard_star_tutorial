CONFIG += testcase
TARGET = tst_qqmlpropertycache
macx:CONFIG -= app_bundle

SOURCES += tst_qqmlpropertycache.cpp

include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml-private testlib
