CONFIG += testcase

TARGET = tst_qquickdraghandler
QT += core-private gui-private qml-private quick-private  testlib

macos:CONFIG -= app_bundle

SOURCES  += tst_qquickdraghandler.cpp

include (../../../shared/util.pri)
include (../../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += data/DragAnywhereSlider.qml \
    data/FlashAnimation.qml \
    data/Slider.qml \
    data/draggables.qml \
    data/grabberstate.qml \
    data/multipleSliders.qml \
    data/reparenting.qml \
