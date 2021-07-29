TEMPLATE = app

TARGET = tst_armature

QT += core-private 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += \
    tst_armature.cpp

include(../../core/common/common.pri)
