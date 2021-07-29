QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += vulkan-server.json

SOURCES += \
    main.cpp

include($PWD/../../../../../hardwareintegration/compositor/vulkan-server/vulkan-server.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = VulkanServerBufferIntegrationPlugin
load(qt_plugin)
