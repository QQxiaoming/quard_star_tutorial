TEMPLATE = app
QT       += remoteobjects core testlib
QT       -= gui

TARGET = server
DESTDIR = ./
CONFIG   += c++11
CONFIG   -= app_bundle

REPC_SOURCE = $$PWD/../subclass.rep

SOURCES += main.cpp \
    mytestserver.cpp

HEADERS += \
    ../shared.h \
    ../namespace.h \
    mytestserver.h \
    $$OUT_PWD/rep_subclass_source.h

INCLUDEPATH += $$PWD
