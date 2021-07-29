TARGET = qtposition_cl

QT = core core-private positioning

OBJECTIVE_SOURCES += \
    qgeopositioninfosource_cl.mm \
    qgeopositioninfosourcefactory_cl.mm

HEADERS += \
    qgeopositioninfosource_cl_p.h \
    qgeopositioninfosourcefactory_cl.h

OTHER_FILES += \
    plugin.json

LIBS += -framework Foundation -framework CoreLocation

!darwin {
    DISTFILES += $$OBJECTIVE_SOURCES
}

PLUGIN_TYPE = position
PLUGIN_CLASS_NAME = QGeoPositionInfoSourceFactoryCL
load(qt_plugin)
