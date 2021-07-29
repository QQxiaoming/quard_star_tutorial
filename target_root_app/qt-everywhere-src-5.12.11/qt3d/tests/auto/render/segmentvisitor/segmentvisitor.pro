TEMPLATE = app

TARGET = segmentvisitor

QT += 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += tst_segmentvisitor.cpp

include(../../core/common/common.pri)
include(../commons/commons.pri)
