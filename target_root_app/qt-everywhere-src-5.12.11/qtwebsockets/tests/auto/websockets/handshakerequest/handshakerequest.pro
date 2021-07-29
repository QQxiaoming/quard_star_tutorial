CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_handshakerequest

QT = testlib websockets-private

SOURCES += tst_handshakerequest.cpp

requires(qtConfig(private_tests))
