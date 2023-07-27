LIBS += -lz

HEADERS += \
    $$PWD/byteswap-osx.h \
    $$PWD/common.h \
    $$PWD/jffs2.h \
    $$PWD/jffs2-user.h \
    $$PWD/lzoconf.h \
    $$PWD/lzodefs.h \
    $$PWD/minilzo.h \
    $$PWD/xalloc.h

SOURCES += \
    $$PWD/jffs2extract.cpp \
    $$PWD/minilzo.cpp

INCLUDEPATH += $$PWD
