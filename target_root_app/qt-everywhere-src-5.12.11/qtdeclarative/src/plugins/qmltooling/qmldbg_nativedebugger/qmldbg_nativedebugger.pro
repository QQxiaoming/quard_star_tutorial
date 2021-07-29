TARGET = qmldbg_nativedebugger
QT = qml-private core packetprotocol-private

SOURCES += \
    $$PWD/qqmlnativedebugservicefactory.cpp \
    $$PWD/qqmlnativedebugservice.cpp

HEADERS += \
    $$PWD/qqmlnativedebugservicefactory.h \
    $$PWD/qqmlnativedebugservice.h \

OTHER_FILES += \
    $$PWD/qqmlnativedebugservice.json

PLUGIN_TYPE = qmltooling
PLUGIN_CLASS_NAME = QQmlNativeDebugServiceFactory
load(qt_plugin)
