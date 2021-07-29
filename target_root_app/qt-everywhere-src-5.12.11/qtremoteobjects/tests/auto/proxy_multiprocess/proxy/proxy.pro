TEMPLATE = app
QT       += remoteobjects core testlib
QT       -= gui

TARGET = proxy
DESTDIR = ./
CONFIG   += c++11
CONFIG   -= app_bundle

SOURCES += main.cpp \

HEADERS += \

INCLUDEPATH += $$PWD
