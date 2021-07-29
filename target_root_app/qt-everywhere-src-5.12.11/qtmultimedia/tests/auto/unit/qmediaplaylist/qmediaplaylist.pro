CONFIG += testcase
TARGET = tst_qmediaplaylist

include (../qmultimedia_common/mockplaylist.pri)

QT += multimedia-private testlib

SOURCES += \
    tst_qmediaplaylist.cpp

INCLUDEPATH += ../../../../src/plugins/m3u

TESTDATA += testdata/*
