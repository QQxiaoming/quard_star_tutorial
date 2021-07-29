CXX_MODULE = qml
TARGET  = particlesplugin
TARGETPATH = QtQuick/Particles.2
IMPORT_VERSION = 2.$$QT_MINOR_VERSION

SOURCES += \
    plugin.cpp

QT += quick-private quickparticles-private qml-private

load(qml_plugin)
