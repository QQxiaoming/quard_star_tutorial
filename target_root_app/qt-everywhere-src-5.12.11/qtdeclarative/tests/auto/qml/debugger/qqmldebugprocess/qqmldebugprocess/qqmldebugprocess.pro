CONFIG += testcase
TARGET = tst_qqmldebugprocess
QT = core testlib
CONFIG -= debug_and_release_target
macos:CONFIG -= app_bundle

SOURCES += \
    ../../shared/qqmldebugprocess.cpp \
    tst_qqmldebugprocess.cpp

HEADERS += \
    ../../shared/qqmldebugprocess_p.h

INCLUDEPATH += ../../shared
