CXX_MODULE = qml
TARGET  = qmlwavefrontmeshplugin
TARGETPATH = Qt/labs/wavefrontmesh
IMPORT_VERSION = 1.$$QT_MINOR_VERSION

QT = core-private qml-private quick-private

SOURCES += \
    plugin.cpp \
    qwavefrontmesh.cpp

HEADERS += \
    qwavefrontmesh.h

load(qml_plugin)
