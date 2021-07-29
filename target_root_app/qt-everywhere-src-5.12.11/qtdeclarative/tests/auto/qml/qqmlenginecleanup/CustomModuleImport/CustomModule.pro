TEMPLATE = lib
TARGET = CustomModule
QT += quick qml

CONFIG += qtquickcompiler
SOURCES += moduleplugin.cpp
RESOURCES += moduleplugin.qrc

DESTDIR = ../CustomModule

IMPORT_FILES = qmldir

include (../../../shared/imports.pri)
