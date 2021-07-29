# We have a bunch of C code with casts, so we can't have this option
QMAKE_CXXFLAGS_WARN_ON -= -Wcast-qual

QT += waylandclient-private

include(../../../../hardwareintegration/client/dmabuf-server/dmabuf-server.pri)

OTHER_FILES += \
    dmabuf-server.json

SOURCES += main.cpp

PLUGIN_TYPE = wayland-graphics-integration-client
PLUGIN_CLASS_NAME = DmaBufServerBufferPlugin
load(qt_plugin)
