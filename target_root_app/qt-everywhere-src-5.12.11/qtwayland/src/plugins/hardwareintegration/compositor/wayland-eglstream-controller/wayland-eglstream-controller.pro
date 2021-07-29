QT = waylandcompositor waylandcompositor-private core-private gui-private

OTHER_FILES += wayland-eglstream-controller.json

SOURCES += \
    main.cpp \

include(../../../../hardwareintegration/compositor/wayland-eglstream-controller/wayland-eglstream-controller.pri)

PLUGIN_TYPE = wayland-graphics-integration-server
PLUGIN_CLASS_NAME = QWaylandEglStreamBufferIntegrationPlugin
load(qt_plugin)
