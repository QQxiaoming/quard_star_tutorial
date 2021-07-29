TARGET = tst_qheif

QT = core gui testlib
CONFIG -= app_bundle
CONFIG += testcase

SOURCES += tst_qheif.cpp
RESOURCES += $$PWD/../../shared/images/heif.qrc
