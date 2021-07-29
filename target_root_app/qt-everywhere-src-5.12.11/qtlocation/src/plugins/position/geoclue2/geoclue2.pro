TARGET = qtposition_geoclue2

QT = core positioning dbus

HEADERS += \
    qgeopositioninfosource_geoclue2_p.h \
    qgeopositioninfosourcefactory_geoclue2.h \
    geocluetypes.h

SOURCES += \
    qgeopositioninfosource_geoclue2.cpp \
    qgeopositioninfosourcefactory_geoclue2.cpp \
    geocluetypes.cpp

QDBUSXML2CPP_INTERFACE_HEADER_FLAGS += \
    "-N -i geocluetypes.h"

DBUS_INTERFACES += \
    org.freedesktop.GeoClue2.Manager.xml \
    org.freedesktop.GeoClue2.Client.xml \
    org.freedesktop.GeoClue2.Location.xml

INCLUDEPATH += $$QT.location.includes $$OUT_PWD

OTHER_FILES += \
    plugin.json

PLUGIN_TYPE = position
PLUGIN_CLASS_NAME = QGeoPositionInfoSourceFactoryGeoclue2
load(qt_plugin)
