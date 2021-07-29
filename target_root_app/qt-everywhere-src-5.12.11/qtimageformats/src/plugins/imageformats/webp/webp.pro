TARGET  = qwebp

HEADERS += qwebphandler_p.h
SOURCES += main.cpp qwebphandler.cpp
OTHER_FILES += webp.json

include($$OUT_PWD/../../../imageformats/qtimageformats-config.pri)
QT_FOR_CONFIG += imageformats-private

qtConfig(system-webp) {
    QMAKE_USE += webp
} else {
    include($$PWD/../../../3rdparty/libwebp.pri)
}

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QWebpPlugin
load(qt_plugin)
