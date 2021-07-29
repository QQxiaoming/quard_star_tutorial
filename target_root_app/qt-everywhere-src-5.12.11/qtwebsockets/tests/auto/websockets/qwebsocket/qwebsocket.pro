CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

QT = testlib websockets

TARGET = tst_qwebsocket

TEMPLATE = app

SOURCES += tst_qwebsocket.cpp
