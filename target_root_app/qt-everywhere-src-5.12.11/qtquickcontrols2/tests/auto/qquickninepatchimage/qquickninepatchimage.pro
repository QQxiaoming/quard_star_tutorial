CONFIG += testcase
macos:CONFIG -= app_bundle
TARGET = tst_qquickninepatchimage

QT += core gui qml quick testlib
QT_PRIVATE += gui-private quick-private quickcontrols2-private

include (../shared/util.pri)

SOURCES += tst_qquickninepatchimage.cpp

TESTDATA += \
    $$PWD/data/*.qml \
    $$PWD/data/*.png
