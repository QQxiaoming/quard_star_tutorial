TEMPLATE = subdirs

qtHaveModule(location) {

    #Place unit tests
    SUBDIRS += qplace \
           qplaceattribute \
           qplacecategory \
           qplacecontactdetail \
           qplacecontentrequest \
           qplacedetailsreply \
           qplaceeditorial \
           qplacematchreply \
           qplacematchrequest \
           qplaceimage \
           qplaceratings \
           qplaceresult \
           qproposedsearchresult \
           qplacereply \
           qplacereview \
           qplacesearchrequest \
           qplacesupplier \
           qplacesearchresult \
           qplacesearchreply \
           qplacesearchsuggestionreply \
           qplaceuser

    !android: SUBDIRS += \
           qplacemanager \
           qplacemanager_nokia \
           qplacemanager_unsupported \
           placesplugin_unsupported

    #misc tests
    SUBDIRS +=   doublevectors
    !android: SUBDIRS += cmake qmlinterface # looks for .qmls locally

    #Map and Navigation tests
    SUBDIRS += geotestplugin \
           qgeocodingmanagerplugins \
           qgeocameracapabilities\
           qgeocameradata \
           qgeocodereply \
           qgeomaneuver \
           qgeotiledmapscene \
           qgeoroute \
           qgeoroutereply \
           qgeorouterequest \
           qgeoroutesegment \
           qgeoroutingmanagerplugins \
           qgeotilespec \
           qgeoroutexmlparser \
           maptype \
           qgeocameratiles

    # These use plugins
    !android: SUBDIRS += qgeoserviceprovider \
                         qgeoroutingmanager \
                         nokia_services \
                         qgeocodingmanager \
                         qgeotiledmap

    qtHaveModule(quick):!android {
        SUBDIRS += declarative_geoshape \
                   declarative_core

        !mac: SUBDIRS += declarative_ui
    }
}


SUBDIRS += \
           qgeoaddress \
           qgeoshape \
           qgeorectangle \
           qgeocircle \
           qgeopath \
           qgeopolygon \
           qgeocoordinate \
           qgeolocation \
           qgeopositioninfo \
           qgeosatelliteinfo \

!android: SUBDIRS += \
            positionplugin \
            positionplugintest \
            qgeoareamonitor \
            qgeopositioninfosource \
            qgeosatelliteinfosource \
            qnmeapositioninfosource
