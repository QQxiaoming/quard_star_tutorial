# We have a bunch of C code with casts, so we can't have this option
QMAKE_CXXFLAGS_WARN_ON -= -Wcast-qual

QT += waylandclient-private

include(../../../../hardwareintegration/client/vulkan-server/vulkan-server.pri)

OTHER_FILES += \
    vulkan-server.json

SOURCES += main.cpp

PLUGIN_TYPE = wayland-graphics-integration-client
PLUGIN_CLASS_NAME = VulkanServerBufferPlugin
load(qt_plugin)
