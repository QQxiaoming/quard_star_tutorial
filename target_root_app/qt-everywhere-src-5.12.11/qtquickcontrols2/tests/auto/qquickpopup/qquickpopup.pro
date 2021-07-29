CONFIG += testcase
TARGET = tst_qquickpopup
SOURCES += tst_qquickpopup.cpp

macos:CONFIG -= app_bundle

QT += core-private gui-private qml-private quick-private testlib quicktemplates2-private

include (../shared/util.pri)

TESTDATA = data/*

OTHER_FILES += \
    data/*.qml
