TEMPLATE = app

TARGET = tst_qjoint

QT += 3dcore 3dcore-private testlib

CONFIG += testcase

SOURCES += \
    tst_qjoint.cpp

include(../common/common.pri)
