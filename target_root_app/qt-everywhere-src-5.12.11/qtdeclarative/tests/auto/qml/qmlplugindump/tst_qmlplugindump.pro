QT += testlib gui-private qml
macx:CONFIG -= app_bundle

CONFIG += testcase

include(../../shared/util.pri)

DEFINES += QT_QMLTEST_DIR=\\\"$${_PRO_FILE_PWD_}\\\"
SOURCES += tst_qmlplugindump.cpp
