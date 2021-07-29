TEMPLATE = app
QT       += remoteobjects core testlib
QT       -= gui

TARGET = client
DESTDIR = ./
CONFIG   += c++11
CONFIG   -= app_bundle

REPC_REPLICA = ../subclass.rep

SOURCES += main.cpp \

HEADERS += \
    ../shared.h \
    ../namespace.h

INCLUDEPATH += $$PWD
