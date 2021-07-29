CXX_MODULE = qml
TARGET  = qwaylandtexturesharingextension
TARGETPATH = QtWayland/Compositor/TextureSharingExtension
IMPORT_VERSION = 1.$$QT_MINOR_VERSION

SOURCES += \
    plugin.cpp

QT += quick-private qml gui-private core-private waylandcompositor waylandcompositor-private

requires(qtConfig(wayland-compositor-texture-sharing-experimental))

load(qml_plugin)
