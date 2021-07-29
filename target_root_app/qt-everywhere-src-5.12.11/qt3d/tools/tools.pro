TEMPLATE = subdirs

!qtHaveModule(3dcore): \
    return()

QT_FOR_CONFIG += 3dcore-private
qtConfig(assimp):qtConfig(commandlineparser): {
    SUBDIRS += qgltf
}
