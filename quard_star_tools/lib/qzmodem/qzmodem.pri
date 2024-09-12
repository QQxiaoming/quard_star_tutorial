INCLUDEPATH += \
        -I $$PWD/util \
        -I $$PWD 

SOURCES +=                  \
    $$PWD/util/crctab.cpp   \
    $$PWD/util/timing.cpp   \
    $$PWD/lowlevelstuff.cpp \
    $$PWD/qrecvzmodem.cpp   \
    $$PWD/qsendzmodem.cpp   

HEADERS +=                \
    $$PWD/util/crctab.h   \
    $$PWD/util/zglobal.h  \
    $$PWD/util/timing.h   \
    $$PWD/lowlevelstuff.h \
    $$PWD/qrecvzmodem.h   \
    $$PWD/qsendzmodem.h   

#DEFINES += DEBUGZ
