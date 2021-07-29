CONFIG += testcase
TARGET = tst_qmldiskcache
osx:CONFIG -= app_bundle

SOURCES += tst_qmldiskcache.cpp

RESOURCES += test.qml importmodule.qml module.mjs

QT += core-private qml-private testlib
