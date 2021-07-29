CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_websockets

QT = websockets testlib

SOURCES += tst_websockets.cpp
