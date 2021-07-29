TEMPLATE = subdirs

QT_FOR_CONFIG += location-private  # pulls in the features defined in configure.json

qtConfig(geoservices_here): SUBDIRS += nokia
qtConfig(geoservices_mapbox): SUBDIRS += mapbox
qtConfig(geoservices_esri): SUBDIRS += esri
qtConfig(geoservices_itemsoverlay): SUBDIRS += itemsoverlay
qtConfig(geoservices_osm): SUBDIRS += osm

qtConfig(geoservices_mapboxgl) {
    !exists(../../3rdparty/mapbox-gl-native/mapbox-gl-native.pro) {
        warning("Submodule mapbox-gl-native does not exist. Run 'git submodule update --init' on qtlocation.")
    } else {
        SUBDIRS += mapboxgl ../../3rdparty/mapbox-gl-native
        mapboxgl.depends = ../../3rdparty/mapbox-gl-native
    }
}
