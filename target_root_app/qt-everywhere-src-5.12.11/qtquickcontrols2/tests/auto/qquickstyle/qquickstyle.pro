CONFIG += testcase
TARGET = tst_qquickstyle
SOURCES += tst_qquickstyle.cpp

macos:CONFIG -= app_bundle

QT += quickcontrols2 testlib
QT_PRIVATE += core-private gui-private quickcontrols2-private

include (../shared/util.pri)

TESTDATA = $$PWD/data/*

qrcStyles1.files = $$files(qrcStyles1/QrcStyle1/*.qml)
qrcStyles1.prefix = /
RESOURCES += qrcStyles1

qrcStyles2.files = $$files(qrcStyles2/QrcStyle2/*.qml)
qrcStyles2.prefix = /
RESOURCES += qrcStyles2

qrcStyles3.files = $$files(qrcStyles3/QrcStyle3/*.qml)
qrcStyles3.prefix = /
RESOURCES += qrcStyles3

qrcStyles4.files = $$files(qrcStyles4/QrcStyle4/*.qml)
qrcStyles4.prefix = /
RESOURCES += qrcStyles4
