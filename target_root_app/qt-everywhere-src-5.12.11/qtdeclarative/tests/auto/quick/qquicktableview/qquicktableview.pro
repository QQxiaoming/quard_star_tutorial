CONFIG += testcase
TARGET = tst_qquicktableview
macos:CONFIG -= app_bundle

HEADERS += testmodel.h
SOURCES += tst_qquicktableview.cpp

include (../../shared/util.pri)
include (../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml-private quick-private testlib

