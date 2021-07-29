QT += qmldebug-private

INCLUDEPATH += $$PWD
include($$PWD/../../../shared/util.pri)

HEADERS += \
    $$PWD/debugutil_p.h \
    $$PWD/qqmldebugprocess_p.h
SOURCES += \
    $$PWD/debugutil.cpp \
    $$PWD/qqmldebugprocess.cpp
