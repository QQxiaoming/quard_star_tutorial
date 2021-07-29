TEMPLATE = app

TARGET = tst_qcallbackmapping

QT += 3dcore 3dcore-private 3danimation 3danimation-private testlib

CONFIG += testcase

SOURCES += \
    tst_qcallbackmapping.cpp

include(../../core/common/common.pri)
