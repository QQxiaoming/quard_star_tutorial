HEADERS += \
    $$PWD/byteswap-osx.h \
    $$PWD/common.h \
    $$PWD/jffs2.h \
    $$PWD/jffs2-user.h \
    $$PWD/lzoconf.h \
    $$PWD/lzodefs.h \
    $$PWD/minilzo.h \
    $$PWD/xalloc.h \
    $$PWD/byteswap.h \
    $$PWD/crc32.h \
    $$PWD/endian.h

SOURCES += \
    $$PWD/jffs2extract.cpp \
    $$PWD/mini_inflate.cpp \
    $$PWD/libcrc32.cpp \
    $$PWD/minilzo.cpp

INCLUDEPATH += $$PWD
