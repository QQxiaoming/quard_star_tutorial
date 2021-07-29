CONFIG += testcase c++11
CONFIG -= app_bundle
TARGET = tst_proxy_multiprocess
DESTDIR = ./
QT += testlib remoteobjects
QT -= gui

SOURCES += tst_proxy_multiprocess.cpp
