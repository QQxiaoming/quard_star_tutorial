TARGET = qtgeoservices_esri

QT += location-private positioning-private network

QT_FOR_CONFIG += location-private
qtConfig(location-labs-plugin): DEFINES += LOCATIONLABS

HEADERS += \
    geocodereply_esri.h \
    geocodingmanagerengine_esri.h \
    geomapsource.h \
    georoutejsonparser_esri.h \
    georoutereply_esri.h \
    georoutingmanagerengine_esri.h \
    geoserviceproviderfactory_esri.h \
    geotiledmap_esri.h \
    geotiledmappingmanagerengine_esri.h \
    geotiledmapreply_esri.h \
    geotilefetcher_esri.h \
    placemanagerengine_esri.h \
    placesearchreply_esri.h \
    placecategoriesreply_esri.h

SOURCES += \
    geocodereply_esri.cpp  \
    geocodingmanagerengine_esri.cpp \
    geomapsource.cpp \
    georoutejsonparser_esri.cpp \
    georoutereply_esri.cpp \
    georoutingmanagerengine_esri.cpp \
    geoserviceproviderfactory_esri.cpp \
    geotiledmap_esri.cpp \
    geotiledmappingmanagerengine_esri.cpp \
    geotiledmapreply_esri.cpp \
    geotilefetcher_esri.cpp \
    placemanagerengine_esri.cpp \
    placesearchreply_esri.cpp \
    placecategoriesreply_esri.cpp

RESOURCES += \
    esri.qrc

OTHER_FILES += \
    esri_plugin.json

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = GeoServiceProviderFactoryEsri
load(qt_plugin)
