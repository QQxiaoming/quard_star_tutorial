CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_dataprocessor

QT = testlib websockets-private

SOURCES += tst_dataprocessor.cpp

requires(qtConfig(private_tests))
