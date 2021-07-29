SOURCES += tst_qbluetoothlocaldevice.cpp
TARGET=tst_qbluetoothlocaldevice
CONFIG += testcase

QT = core concurrent bluetooth-private testlib
osx:QT += widgets
