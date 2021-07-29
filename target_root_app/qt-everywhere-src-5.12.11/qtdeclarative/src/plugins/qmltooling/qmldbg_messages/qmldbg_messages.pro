TARGET = qmldbg_messages
QT = qml-private core packetprotocol-private

SOURCES += \
    $$PWD/qdebugmessageservice.cpp \
    $$PWD/qdebugmessageservicefactory.cpp

HEADERS += \
    $$PWD/qdebugmessageservice.h \
    $$PWD/qdebugmessageservicefactory.h

OTHER_FILES += \
    $$PWD/qdebugmessageservice.json

PLUGIN_TYPE = qmltooling
PLUGIN_CLASS_NAME = QDebugMessageServiceFactory
load(qt_plugin)
