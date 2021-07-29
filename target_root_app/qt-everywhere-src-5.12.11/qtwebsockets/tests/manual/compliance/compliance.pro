CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_compliance

QT = websockets testlib

SOURCES += tst_compliance.cpp
