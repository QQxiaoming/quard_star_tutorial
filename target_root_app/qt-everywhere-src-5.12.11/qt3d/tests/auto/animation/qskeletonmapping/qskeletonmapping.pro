TEMPLATE = app

TARGET = tst_qskeletonmapping

QT += 3dcore 3dcore-private 3danimation 3danimation-private testlib

CONFIG += testcase

SOURCES += \
    tst_qskeletonmapping.cpp

include(../../core/common/common.pri)
