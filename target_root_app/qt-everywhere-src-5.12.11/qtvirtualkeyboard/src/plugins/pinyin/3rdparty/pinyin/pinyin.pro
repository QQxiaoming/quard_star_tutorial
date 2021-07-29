TARGET = qtpinyin

VERSION = 1.0.0
CONFIG += static
CONFIG += warn_off

MODULE_INCLUDEPATH = $$PWD/include
MODULE_DEFINES += HAVE_PINYIN

SOURCES += \
    share/dictbuilder.cpp \
    share/dictlist.cpp \
    share/dicttrie.cpp \
    share/lpicache.cpp \
    share/matrixsearch.cpp \
    share/mystdlib.cpp \
    share/ngram.cpp \
    share/pinyinime.cpp \
    share/searchutility.cpp \
    share/spellingtable.cpp \
    share/spellingtrie.cpp \
    share/splparser.cpp \
    share/sync.cpp \
    share/userdict.cpp \
    share/utf16char.cpp \
    share/utf16reader.cpp

HEADERS += \
    include/atomdictbase.h \
    include/dictbuilder.h \
    include/dictdef.h \
    include/dictlist.h \
    include/dicttrie.h \
    include/lpicache.h \
    include/matrixsearch.h \
    include/mystdlib.h \
    include/ngram.h \
    include/pinyinime.h \
    include/searchutility.h \
    include/spellingtable.h \
    include/spellingtrie.h \
    include/splparser.h \
    include/sync.h \
    include/userdict.h \
    include/utf16char.h \
    include/utf16reader.h

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

OTHER_FILES +=\
    data/rawdict_utf16_65105_freq.txt \
    data/valid_utf16.txt

load(qt_helper_lib)

CONFIG += qt
QT = core
!win32 {
    CONFIG *= thread
}
