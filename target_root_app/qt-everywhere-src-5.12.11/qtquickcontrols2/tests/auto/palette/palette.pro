CONFIG += testcase
TARGET = tst_palette
SOURCES += tst_palette.cpp

macos:CONFIG -= app_bundle

QT += core-private gui-private qml-private quick-private testlib quicktemplates2-private quickcontrols2-private

include (../shared/util.pri)

RESOURCES += qtquickcontrols2.conf

TESTDATA = data/*

OTHER_FILES += \
    data/*.qml
