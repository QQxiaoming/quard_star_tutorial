TEMPLATE = subdirs

QT_FOR_CONFIG += 3dcore

qtConfig(qt3d-render):qtConfig(qt3d-input):qtConfig(qt3d-extras):qtConfig(qt3d-render):qtConfig(qt3d-logic) {
    SUBDIRS += \
        aspects_startup_shutdown
}
