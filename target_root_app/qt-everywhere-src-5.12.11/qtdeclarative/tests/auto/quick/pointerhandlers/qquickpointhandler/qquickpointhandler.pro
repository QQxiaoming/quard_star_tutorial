CONFIG += testcase

TARGET = tst_qquickpointhandler
QT += core-private gui-private qml-private quick-private testlib

macos:CONFIG -= app_bundle

SOURCES  += tst_qquickpointhandler.cpp

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += data/pointTracker.qml \
