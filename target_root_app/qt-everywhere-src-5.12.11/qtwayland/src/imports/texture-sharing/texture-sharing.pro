CXX_MODULE = qml
TARGET  = qwaylandtexturesharing
TARGETPATH = QtWayland/Client/TextureSharing
IMPORT_VERSION = 1.$$QT_MINOR_VERSION

HEADERS += \
    sharedtextureprovider.h \
    texturesharingextension.h

SOURCES += \
    plugin.cpp \
    sharedtextureprovider.cpp \
    texturesharingextension.cpp

QT += quick-private qml gui-private core-private waylandclient waylandclient-private
CONFIG += wayland-scanner

requires(qtConfig(wayland-client-texture-sharing-experimental))

WAYLANDCLIENTSOURCES += ../../extensions/qt-texture-sharing-unstable-v1.xml


load(qml_plugin)
