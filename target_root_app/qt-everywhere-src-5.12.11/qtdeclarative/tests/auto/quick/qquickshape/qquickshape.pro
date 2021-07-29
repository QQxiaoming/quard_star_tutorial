CONFIG += testcase
TARGET = tst_qquickshape
macos:CONFIG -= app_bundle

SOURCES += tst_qquickshape.cpp

include (../../shared/util.pri)
include (../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private  qml-private quick-private testlib quickshapes-private
qtHaveModule(widgets): QT += widgets
