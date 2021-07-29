CONFIG += testcase
TARGET = tst_qqmlapplicationengine
macx:CONFIG -= app_bundle


SOURCES += tst_qqmlapplicationengine.cpp
TESTDATA += data/*
RESOURCES += tst_qqmlapplicationengine.qrc

include (../../shared/util.pri)
QT += core-private gui-private qml-private network testlib

TRANSLATIONS = data/i18n/qml_ja.ts
