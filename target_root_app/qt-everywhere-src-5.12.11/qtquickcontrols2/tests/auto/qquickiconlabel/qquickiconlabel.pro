CONFIG += testcase
macos:CONFIG -= app_bundle
TARGET = tst_qquickiconlabel

QT += core gui qml quick testlib
QT_PRIVATE += quick-private quickcontrols2-private

include (../shared/util.pri)

SOURCES += tst_qquickiconlabel.cpp

TESTDATA += \
    $$PWD/data/*.qml
