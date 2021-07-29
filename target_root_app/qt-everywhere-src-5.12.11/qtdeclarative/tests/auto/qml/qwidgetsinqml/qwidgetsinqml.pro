CONFIG += testcase
CONFIG += parallel_test
TARGET = tst_qwidgetsinqml
macos:CONFIG -= app_bundle
QT += qml widgets testlib gui-private
SOURCES  += tst_qwidgetsinqml.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
