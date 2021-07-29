CONFIG += testcase
TARGET = tst_qquickapplicationwindow
SOURCES += tst_qquickapplicationwindow.cpp

macos:CONFIG -= app_bundle

QT += core-private gui-private qml-private quick-private quicktemplates2-private quickcontrols2-private testlib

include (../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += \
    data/*.qml

