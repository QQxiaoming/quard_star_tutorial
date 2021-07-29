CONFIG += testcase
TARGET = tst_qdebugmessageservice
QT += network testlib gui-private core-private
osx:CONFIG -= app_bundle

SOURCES +=     tst_qdebugmessageservice.cpp

include(../shared/debugutil.pri)

TESTDATA = data/*

OTHER_FILES += data/test.qml
