CONFIG += testcase
TARGET = tst_qv4mm
include (../../shared/util.pri)

macos:CONFIG -= app_bundle

TESTDATA = data/*

SOURCES += tst_qv4mm.cpp

QT += qml qml-private testlib

