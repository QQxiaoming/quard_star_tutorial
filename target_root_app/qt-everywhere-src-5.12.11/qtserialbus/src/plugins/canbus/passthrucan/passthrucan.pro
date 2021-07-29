QT = serialbus

TARGET = qtpassthrucanbus

SOURCES += \
    j2534passthru.cpp \
    main.cpp \
    passthrucanio.cpp \
    passthrucanbackend.cpp

HEADERS += \
    j2534passthru.h \
    passthrucanio.h \
    passthrucanbackend.h

DISTFILES = plugin.json

PLUGIN_TYPE = canbus
PLUGIN_CLASS_NAME = PassThruCanBusPlugin
load(qt_plugin)
