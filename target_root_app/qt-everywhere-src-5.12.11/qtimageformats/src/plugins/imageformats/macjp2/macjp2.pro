TARGET  = qmacjp2
PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QMacJp2Plugin

LIBS += -framework CoreFoundation -framework CoreGraphics -framework ImageIO

QT += core-private gui-private

SOURCES += \
    qmacjp2handler.cpp \
    main.cpp

HEADERS += \
    qmacjp2handler.h

include (../shared/qiiofhelpers.pri)

OTHER_FILES += macjp2.json

load(qt_plugin)
