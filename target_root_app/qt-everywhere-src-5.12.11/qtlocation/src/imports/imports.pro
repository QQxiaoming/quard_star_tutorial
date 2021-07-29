TEMPLATE = subdirs

QT_FOR_CONFIG += location-private

qtHaveModule(positioningquick): SUBDIRS += positioning
qtHaveModule(location): SUBDIRS += location
qtHaveModule(location):qtConfig(location-labs-plugin): SUBDIRS += locationlabs
