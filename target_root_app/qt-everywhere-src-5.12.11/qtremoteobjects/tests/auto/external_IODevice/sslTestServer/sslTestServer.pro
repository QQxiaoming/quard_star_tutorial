TEMPLATE = app
QT       += remoteobjects core testlib
QT       -= gui

TARGET = sslTestServer

DESTDIR = ./
CONFIG   += c++11
CONFIG   -= app_bundle

INCLUDEPATH += $$PWD

REPC_SOURCE = $$PWD/../pingpong.rep

RESOURCES += $$PWD/../cert/cert.qrc

SOURCES += main.cpp \
    pingpong.cpp \
    sslserver.cpp

HEADERS += \
    pingpong.h \
    sslserver.h
