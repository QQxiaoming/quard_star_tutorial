CONFIG += testcase
TARGET = tst_qqmldebuggingenabler
QT += qml testlib gui-private core-private
CONFIG -= debug_and_release_target
osx:CONFIG -= app_bundle

SOURCES += tst_qqmldebuggingenabler.cpp

include(../../shared/debugutil.pri)

OTHER_FILES += data/test.qml
