TARGET = qttcime

CONFIG += static

SOURCES += \
    cangjiedictionary.cpp \
    cangjietable.cpp \
    phrasedictionary.cpp \
    worddictionary.cpp \
    zhuyindictionary.cpp \
    zhuyintable.cpp

HEADERS += \
    cangjiedictionary.h \
    cangjietable.h \
    phrasedictionary.h \
    worddictionary.h \
    zhuyindictionary.h \
    zhuyintable.h

OTHER_FILES += \
    data/dict_cangjie.dat \
    data/dict_phrases.dat

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

MODULE_INCLUDEPATH = $$PWD
MODULE_DEFINES = HAVE_TCIME

load(qt_helper_lib)

CONFIG += qt
QT = core
