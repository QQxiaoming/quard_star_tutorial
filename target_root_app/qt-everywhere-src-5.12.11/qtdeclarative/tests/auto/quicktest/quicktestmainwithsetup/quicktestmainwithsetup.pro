CONFIG += qmltestcase
macos:CONFIG -= app_bundle
TARGET = tst_quicktestmainwithsetup

QT += testlib quick

include (../../shared/util.pri)

SOURCES += tst_quicktestmainwithsetup.cpp

TESTDATA += \
    $$PWD/data/*.qml
