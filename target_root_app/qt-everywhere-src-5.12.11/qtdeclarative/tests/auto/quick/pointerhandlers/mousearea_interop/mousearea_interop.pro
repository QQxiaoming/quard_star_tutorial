CONFIG += testcase

TARGET = tst_mousearea_interop
QT += core-private gui-private qml-private quick-private  testlib

macos:CONFIG -= app_bundle

SOURCES  += tst_mousearea_interop.cpp

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += data/dragTakeOverFromSibling.qml
