CONFIG += testcase

TARGET = tst_flickableinterop
QT += core-private gui-private qml-private quick-private  testlib

macos:CONFIG -= app_bundle

SOURCES  += tst_flickableinterop.cpp

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += data/flickableWithHandlers.qml data/Slider.qml
