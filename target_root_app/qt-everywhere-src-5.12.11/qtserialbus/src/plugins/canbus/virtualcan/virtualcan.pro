TARGET = qtvirtualcanbus

QT = core network serialbus

HEADERS += \
    virtualcanbackend.h

SOURCES += \
    main.cpp \
    virtualcanbackend.cpp

DISTFILES = plugin.json

PLUGIN_TYPE = canbus
PLUGIN_CLASS_NAME = VirtualCanBusPlugin
load(qt_plugin)
