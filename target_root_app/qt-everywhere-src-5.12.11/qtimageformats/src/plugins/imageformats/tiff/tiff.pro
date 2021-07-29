TARGET  = qtiff

HEADERS += qtiffhandler_p.h
SOURCES += main.cpp qtiffhandler.cpp
OTHER_FILES += tiff.json

include($$OUT_PWD/../../../imageformats/qtimageformats-config.pri)
QT_FOR_CONFIG += imageformats-private

qtConfig(system-tiff) {
    QMAKE_USE_PRIVATE += tiff
} else {
    include($$PWD/../../../3rdparty/libtiff.pri)
}

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QTiffPlugin
load(qt_plugin)
