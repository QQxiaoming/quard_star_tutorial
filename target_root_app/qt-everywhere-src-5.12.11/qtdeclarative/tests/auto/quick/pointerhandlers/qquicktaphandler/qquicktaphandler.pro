CONFIG += testcase

TARGET = tst_qquicktaphandler
QT += core-private gui-private qml-private quick-private  testlib

macos:CONFIG -= app_bundle

SOURCES  += tst_qquicktaphandler.cpp

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

# OTHER_FILES += data/foo.qml

