TARGET  = qmacheif
PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QMacHeifPlugin

LIBS += -framework CoreFoundation -framework CoreGraphics -framework ImageIO

QT += core-private gui-private

SOURCES += \
    qmacheifhandler.cpp \
    main.cpp

HEADERS += \
    qmacheifhandler.h

include (../shared/qiiofhelpers.pri)

OTHER_FILES += macheif.json

load(qt_plugin)
