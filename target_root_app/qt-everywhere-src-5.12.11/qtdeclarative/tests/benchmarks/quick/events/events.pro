CONFIG += benchmark
TEMPLATE = app
TARGET = tst_qevents
QT += quick quick-private qml testlib
macos:CONFIG -= app_bundle

SOURCES += tst_events.cpp

include (../../../auto/shared/util.pri)
include (../../../auto/quick/shared/util.pri)

