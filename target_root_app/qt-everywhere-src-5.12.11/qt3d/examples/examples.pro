TEMPLATE = subdirs

!qtHaveModule(3dcore): \
    return()

QT_FOR_CONFIG += 3dcore

qtConfig(qt3d-extras): SUBDIRS += qt3d
