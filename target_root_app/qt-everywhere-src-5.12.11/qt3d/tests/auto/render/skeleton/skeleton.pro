TEMPLATE = app

TARGET = tst_skeleton

QT += core-private 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += \
    tst_skeleton.cpp

include(../../core/common/common.pri)
include(../commons/commons.pri)
