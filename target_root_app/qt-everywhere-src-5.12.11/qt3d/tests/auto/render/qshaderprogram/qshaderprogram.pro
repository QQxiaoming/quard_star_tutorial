TEMPLATE = app

TARGET = tst_qshaderprogram

QT += 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += tst_qshaderprogram.cpp

RESOURCES += \
    shaders.qrc

include(../../core/common/common.pri)
include(../commons/commons.pri)
