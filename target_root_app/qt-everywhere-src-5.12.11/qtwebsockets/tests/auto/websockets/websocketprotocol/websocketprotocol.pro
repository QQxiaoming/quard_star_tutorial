CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_websocketprotocol

QT = testlib websockets-private

SOURCES += tst_websocketprotocol.cpp

requires(qtConfig(private_tests))
