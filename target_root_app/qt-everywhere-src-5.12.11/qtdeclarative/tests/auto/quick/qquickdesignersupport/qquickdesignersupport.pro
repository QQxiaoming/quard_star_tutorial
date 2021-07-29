CONFIG += testcase
TARGET = tst_qquickdesignersupport
SOURCES += tst_qquickdesignersupport.cpp

include (../../shared/util.pri)
include (../shared/util.pri)

osx:CONFIG -= app_bundle

TESTDATA = data/*

QT += core-private gui-private qml-private quick-private testlib

DISTFILES += \
    data/TestComponent.qml \
    data/test.qml \
    data/componentTest.qml \
    data/Component01.qml \
    data/Component02.qml
