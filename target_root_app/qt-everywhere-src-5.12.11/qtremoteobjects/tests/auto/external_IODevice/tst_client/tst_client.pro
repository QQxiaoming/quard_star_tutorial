CONFIG += testcase c++11
CONFIG -= app_bundle
TARGET = tst_external_IODevice
DESTDIR = ./
QT += testlib remoteobjects
QT -= gui

REPC_REPLICA = $$PWD/../pingpong.rep
RESOURCES += $$PWD/../cert/cert.qrc

SOURCES += \
    tst_client.cpp
