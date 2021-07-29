CONFIG += testcase
TARGET = tst_qquickapplication
macx:CONFIG -= app_bundle

SOURCES += tst_qquickapplication.cpp
OTHER_FILES += data/tst_displayname.qml \
    data/tst_platformname.qml

include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml quick qml-private quick-private testlib

