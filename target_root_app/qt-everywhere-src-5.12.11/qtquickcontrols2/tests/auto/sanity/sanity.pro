TEMPLATE = app
TARGET = tst_sanity

QT += qml testlib core-private qml-private
CONFIG += testcase
macos:CONFIG -= app_bundle

include(../../auto/shared/util.pri)

SOURCES += \
    $$PWD/tst_sanity.cpp

OTHER_FILES += \
    $$PWD/BLACKLIST
