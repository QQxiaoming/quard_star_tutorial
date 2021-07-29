TEMPLATE = app

TARGET = tst_qcamera

QT += 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += tst_qcamera.cpp

include(../../core/common/common.pri)
include(../commons/commons.pri)
