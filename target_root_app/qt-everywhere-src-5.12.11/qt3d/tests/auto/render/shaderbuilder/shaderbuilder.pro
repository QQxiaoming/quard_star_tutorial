TEMPLATE = app

TARGET = tst_shaderbuilder

QT += core-private 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += tst_shaderbuilder.cpp
RESOURCES += shaderbuilder.qrc

include(../../core/common/common.pri)
include(../commons/commons.pri)
