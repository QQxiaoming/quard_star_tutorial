CONFIG += testcase
macos:CONFIG -= app_bundle
TARGET = tst_qquickiconimage

QT += core gui qml quick testlib
QT_PRIVATE += quick-private quickcontrols2-private
qtHaveModule(svg): QT += svg

include (../shared/util.pri)

SOURCES += tst_qquickiconimage.cpp

RESOURCES += resources.qrc

TESTDATA += \
    $$PWD/data/*.qml \
    $$PWD/data/icons/*
