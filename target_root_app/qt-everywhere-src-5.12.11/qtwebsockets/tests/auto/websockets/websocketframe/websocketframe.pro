CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_websocketframe

QT = testlib websockets-private

SOURCES += tst_websocketframe.cpp

requires(qtConfig(private_tests))
