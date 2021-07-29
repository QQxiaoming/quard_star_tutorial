TEMPLATE = app

TARGET = tst_updatepropertymapjob

QT += core-private 3dcore 3dcore-private 3danimation 3danimation-private testlib

CONFIG += testcase

SOURCES += \
    tst_updatepropertymapjob.cpp

include(../../core/common/common.pri)

RESOURCES += \
    updatepropertymapjob.qrc
