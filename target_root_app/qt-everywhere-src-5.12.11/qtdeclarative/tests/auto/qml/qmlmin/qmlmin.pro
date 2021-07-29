CONFIG += testcase
TARGET = tst_qmlmin
QT += qml testlib gui-private
macx:CONFIG -= app_bundle

SOURCES += tst_qmlmin.cpp
DEFINES += SRCDIR=\\\"$$PWD\\\"

# Boot2qt is cross compiled but it has sources available
!boot2qt {
   cross_compile: DEFINES += QTEST_CROSS_COMPILED
}
