TARGET  = qmng

HEADERS += qmnghandler_p.h
SOURCES += main.cpp \
           qmnghandler.cpp
OTHER_FILES += mng.json

include($$OUT_PWD/../../../imageformats/qtimageformats-config.pri)
QT_FOR_CONFIG += imageformats-private

QMAKE_USE_PRIVATE += mng

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QMngPlugin
load(qt_plugin)
