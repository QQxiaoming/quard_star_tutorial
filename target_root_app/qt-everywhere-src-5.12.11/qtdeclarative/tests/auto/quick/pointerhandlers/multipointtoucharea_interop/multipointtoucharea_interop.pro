CONFIG += testcase

TARGET = tst_multipointtoucharea_interop
QT += core-private gui-private qml-private quick-private  testlib

macos:CONFIG -= app_bundle

SOURCES  += tst_multipointtoucharea_interop.cpp

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += data/pinchDragMPTA.qml
