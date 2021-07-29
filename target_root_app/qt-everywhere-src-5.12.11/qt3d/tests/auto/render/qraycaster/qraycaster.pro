TEMPLATE = app

TARGET = tst_qraycaster

QT += core-private 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += tst_qraycaster.cpp

include(../../core/common/common.pri)
