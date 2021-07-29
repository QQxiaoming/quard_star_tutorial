TARGET = qtgeoservices_itemsoverlay

QT += location-private positioning-private

QT_FOR_CONFIG += location-private
qtConfig(location-labs-plugin): DEFINES += LOCATIONLABS

HEADERS += \
    qgeomapitemsoverlay.h \
    qgeomappingmanagerengineitemsoverlay.h \
    qgeoserviceproviderpluginitemsoverlay.h

SOURCES += \
    qgeoserviceproviderpluginitemsoverlay.cpp \
    qgeomappingmanagerengineitemsoverlay.cpp \
    qgeomapitemsoverlay.cpp

OTHER_FILES += \
    itemsoverlay_plugin.json

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryItemsOverlay
load(qt_plugin)

