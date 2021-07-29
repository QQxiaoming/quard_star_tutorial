CONFIG += testcase
TARGET = tst_qqmlprofilerservice
osx:CONFIG -= app_bundle

SOURCES += tst_qqmlprofilerservice.cpp

include(../shared/debugutil.pri)

TESTDATA = data/*

QT += testlib gui-private

OTHER_FILES += \
    data/pixmapCacheTest.qml \
    data/controlFromJS.qml \
    data/test.qml \
    data/exit.qml \
    data/scenegraphTest.qml \
    data/TestImage_2x2.png \
    data/signalSourceLocation.qml \
    data/javascript.qml \
    data/timer.qml \
    data/qstr.qml \
    data/memory.qml \
    data/batchOverflow.qml
