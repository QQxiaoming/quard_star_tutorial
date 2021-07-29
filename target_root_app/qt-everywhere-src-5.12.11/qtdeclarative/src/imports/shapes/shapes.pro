CXX_MODULE = qml
TARGET  = qmlshapesplugin
TARGETPATH = QtQuick/Shapes
IMPORT_VERSION = 1.$$QT_MINOR_VERSION

QT = core gui-private qml quick-private quickshapes-private

SOURCES += \
    plugin.cpp \

load(qml_plugin)
