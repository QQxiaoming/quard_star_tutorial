CONFIG += testcase
TARGET = tst_qqmlengine
macx:CONFIG -= app_bundle

include (../../shared/util.pri)

SOURCES += tst_qqmlengine.cpp

QT += core-private gui-private qml-private  network testlib

boot2qt: {
    # GC corruption test is too heavy for qemu-arm
    DEFINES += SKIP_GCCORRUPTION_TEST
}

RESOURCES += \
    data/qrcurls.qml \
    data/qrcurls.js
