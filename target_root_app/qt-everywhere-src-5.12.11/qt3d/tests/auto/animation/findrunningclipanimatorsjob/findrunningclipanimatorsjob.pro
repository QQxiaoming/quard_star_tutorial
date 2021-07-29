TEMPLATE = app

TARGET = tst_findrunningclipanimatorsjob

QT += core-private 3dcore 3dcore-private 3danimation 3danimation-private testlib

CONFIG += testcase

SOURCES += \
    tst_findrunningclipanimatorsjob.cpp

include(../../core/common/common.pri)

RESOURCES += \
    findrunningclipanimatorsjob.qrc
