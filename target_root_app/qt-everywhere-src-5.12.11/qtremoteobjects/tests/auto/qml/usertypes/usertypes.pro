QT       += testlib remoteobjects qml
QT       -= gui

REPC_MERGED += usertypes.rep
#REPC_SOURCE += usertypes.rep

TARGET = tst_usertypes
CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_usertypes.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
