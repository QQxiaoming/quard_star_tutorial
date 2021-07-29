TEMPLATE = app

TARGET = tst_qskeletonloader

QT += 3dcore 3dcore-private testlib

CONFIG += testcase

SOURCES += \
    tst_qskeletonloader.cpp

include(../common/common.pri)
