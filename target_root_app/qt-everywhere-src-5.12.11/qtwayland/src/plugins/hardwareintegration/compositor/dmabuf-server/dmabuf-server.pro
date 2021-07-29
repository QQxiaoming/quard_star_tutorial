QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += dmabuf-server.json

SOURCES += \
    main.cpp

include($PWD/../../../../../hardwareintegration/compositor/dmabuf-server/dmabuf-server.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = DmaBufServerBufferIntegrationPlugin
load(qt_plugin)
