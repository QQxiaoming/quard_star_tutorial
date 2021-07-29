CONFIG += testcase c++11
CONFIG -= app_bundle
TARGET = tst_restart
DESTDIR = ./
QT += testlib remoteobjects
QT -= gui

SOURCES += tst_restart.cpp
