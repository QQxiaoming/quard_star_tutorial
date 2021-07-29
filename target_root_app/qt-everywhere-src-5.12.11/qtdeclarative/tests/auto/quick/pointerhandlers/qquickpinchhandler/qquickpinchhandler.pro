CONFIG += testcase
TARGET = tst_qquickpinchhandler
macos:CONFIG -= app_bundle

SOURCES += tst_qquickpinchhandler.cpp
OTHER_FILES = \
    data/pinchproperties.qml \
    data/threeFingers.qml \
    data/transformedPinchArea.qml

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml-private quick-private testlib
