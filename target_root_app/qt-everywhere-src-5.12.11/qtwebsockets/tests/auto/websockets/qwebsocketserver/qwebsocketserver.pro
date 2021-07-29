CONFIG += console
CONFIG += testcase
CONFIG -= app_bundle

QT = testlib websockets

TARGET = tst_qwebsocketserver

TEMPLATE = app

SOURCES += tst_qwebsocketserver.cpp

RESOURCES += $$PWD/../shared/qwebsocketshared.qrc

boot2qt: DEFINES += SHOULD_CHECK_SYSCALL_SUPPORT
