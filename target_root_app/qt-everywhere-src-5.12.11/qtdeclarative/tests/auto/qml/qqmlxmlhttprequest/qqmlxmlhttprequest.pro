CONFIG += testcase
TARGET = tst_qqmlxmlhttprequest
macx:CONFIG -= app_bundle

HEADERS += ../../shared/testhttpserver.h

SOURCES += tst_qqmlxmlhttprequest.cpp \
           ../../shared/testhttpserver.cpp

include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml-private network testlib
