TARGET = qtgeoservices_mapbox

QT += location-private positioning-private network

QT_FOR_CONFIG += location-private
qtConfig(location-labs-plugin): DEFINES += LOCATIONLABS

HEADERS += \
    qgeoserviceproviderpluginmapbox.h \
    qgeotiledmappingmanagerenginemapbox.h \
    qgeotilefetchermapbox.h \
    qgeomapreplymapbox.h \
    qgeofiletilecachemapbox.h \
    qgeoroutingmanagerenginemapbox.h \
    qgeoroutereplymapbox.h \
    qplacecategoriesreplymapbox.h \
    qplacemanagerenginemapbox.h \
    qplacesearchsuggestionreplymapbox.h \
    qplacesearchreplymapbox.h \
    qgeocodingmanagerenginemapbox.h \
    qgeocodereplymapbox.h \
    qmapboxcommon.h

SOURCES += \
    qgeoserviceproviderpluginmapbox.cpp \
    qgeotiledmappingmanagerenginemapbox.cpp \
    qgeotilefetchermapbox.cpp \
    qgeomapreplymapbox.cpp \
    qgeofiletilecachemapbox.cpp \
    qgeoroutingmanagerenginemapbox.cpp \
    qgeoroutereplymapbox.cpp \
    qplacecategoriesreplymapbox.cpp \
    qplacemanagerenginemapbox.cpp \
    qplacesearchsuggestionreplymapbox.cpp \
    qplacesearchreplymapbox.cpp \
    qgeocodingmanagerenginemapbox.cpp \
    qgeocodereplymapbox.cpp \
    qmapboxcommon.cpp

RESOURCES += mapbox.qrc

OTHER_FILES += \
    mapbox_plugin.json

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryMapbox
load(qt_plugin)
