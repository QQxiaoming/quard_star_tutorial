QT = waylandcompositor waylandcompositor-private core-private gui-private

QMAKE_USE_PRIVATE += wayland-kms

OTHER_FILES += vsp2.json

SOURCES += \
    main.cpp

include($PWD/../../../../../../hardwareintegration/compositor/hardwarelayer/vsp2/vsp2.pri)

PLUGIN_TYPE = wayland-hardware-layer-integration
PLUGIN_CLASS_NAME = Vsp2HardwareLayerIntegrationPlugin
load(qt_plugin)
