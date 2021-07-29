TEMPLATE = app
TARGET = audiorecorder

QT += multimedia

win32:INCLUDEPATH += $$PWD

HEADERS = \
    audiorecorder.h \
    audiolevel.h

SOURCES = \
    main.cpp \
    audiorecorder.cpp \
    audiolevel.cpp

FORMS += audiorecorder.ui

target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/audiorecorder
INSTALLS += target

QT+=widgets
include(../shared/shared.pri)
