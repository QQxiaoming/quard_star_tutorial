QT = core serialbus

SOURCES += main.cpp \
    readtask.cpp \
    canbusutil.cpp \
    sigtermhandler.cpp

HEADERS += \
    readtask.h \
    canbusutil.h \
    sigtermhandler.h

QMAKE_TARGET_DESCRIPTION = "Qt CAN Bus Util"
load(qt_tool)
