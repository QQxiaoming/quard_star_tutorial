TARGET = tst_qaudiodecoderbackend

QT += multimedia multimedia-private testlib

# This is more of a system test
CONFIG += testcase
TESTDATA += testdata/*

INCLUDEPATH += \
    ../../../../src/multimedia/audio

HEADERS += \
    ../shared/mediafileselector.h

SOURCES += \
    tst_qaudiodecoderbackend.cpp

boot2qt: {
    # Yocto sysroot does not have gstreamer/wav
    QMAKE_CXXFLAGS += -DWAV_SUPPORT_NOT_FORCED
}
