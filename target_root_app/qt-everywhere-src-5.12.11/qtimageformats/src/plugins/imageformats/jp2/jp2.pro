TARGET  = qjp2

HEADERS += qjp2handler_p.h
SOURCES += main.cpp \
           qjp2handler.cpp
OTHER_FILES += jp2.json

include($$OUT_PWD/../../../imageformats/qtimageformats-config.pri)
QT_FOR_CONFIG += imageformats-private

QMAKE_USE_PRIVATE += jasper

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QJp2Plugin
load(qt_plugin)
