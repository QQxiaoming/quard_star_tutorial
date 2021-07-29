include(../tests.pri)
QT += webengine webengine-private

HEADERS += \
    server.h \
    testhandler.h

SOURCES += \
    server.cpp \
    testhandler.cpp

RESOURCES += \
    dialogs.qrc
