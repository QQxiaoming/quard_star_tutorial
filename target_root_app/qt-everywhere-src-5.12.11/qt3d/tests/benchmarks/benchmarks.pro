TEMPLATE = subdirs
SUBDIRS = \
    core

QT_FOR_CONFIG += 3dcore

qtConfig(qt3d-render): SUBDIRS += render
