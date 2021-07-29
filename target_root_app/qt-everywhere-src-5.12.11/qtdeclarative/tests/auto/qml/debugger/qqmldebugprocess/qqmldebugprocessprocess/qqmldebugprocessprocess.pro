QT = core
macos:CONFIG -= app_bundle
CONFIG -= debug_and_release_target
CONFIG += console
SOURCES += qqmldebugprocessprocess.cpp

DESTDIR = ../qqmldebugprocess

target.path = $$[QT_INSTALL_TESTS]/tst_qqmldebugprocess
INSTALLS += target

