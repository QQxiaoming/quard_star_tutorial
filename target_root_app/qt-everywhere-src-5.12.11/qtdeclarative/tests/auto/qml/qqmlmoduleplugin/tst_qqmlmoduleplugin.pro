CONFIG += testcase
TARGET = tst_qqmlmoduleplugin

HEADERS = ../../shared/testhttpserver.h
SOURCES = tst_qqmlmoduleplugin.cpp \
          ../../shared/testhttpserver.cpp
CONFIG -= app_bundle

include (../../shared/util.pri)

TESTDATA = data/* imports/* $$OUT_PWD/imports/*

waitingPlugin.files = moduleWithWaitingPlugin
waitingPlugin.prefix = /qt-project.org/imports/
RESOURCES += waitingPlugin

staticPlugin.files = moduleWithStaticPlugin
staticPlugin.prefix = /qt-project.org/imports/
RESOURCES += staticPlugin

QT += core-private gui-private qml-private network testlib
