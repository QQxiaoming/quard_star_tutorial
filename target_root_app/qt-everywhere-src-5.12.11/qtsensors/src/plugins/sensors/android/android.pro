TARGET = qtsensors_android

QT = sensors core-private

# STATICPLUGIN needed because there's a Q_IMPORT_PLUGIN in main.cpp
# Yes, the plugin imports itself statically
DEFINES += QT_STATICPLUGIN

HEADERS = \
    androidaccelerometer.h \
    androidcompass.h \
    androidgyroscope.h \
    androidmagnetometer.h \
    androidpressure.h \
    androidproximity.h \
    androidrotation.h \
    androidtemperature.h \
    androidlight.h \
    sensoreventqueue.h \
    sensormanager.h

SOURCES = \
    main.cpp \
    androidaccelerometer.cpp \
    androidcompass.cpp \
    androidgyroscope.cpp \
    androidmagnetometer.cpp \
    androidpressure.cpp \
    androidproximity.cpp \
    androidrotation.cpp \
    androidtemperature.cpp \
    androidlight.cpp \
    sensormanager.cpp

OTHER_FILES = plugin.json

LIBS += -landroid

PLUGIN_TYPE = sensors
PLUGIN_CLASS_NAME = QCounterGesturePlugin
load(qt_plugin)
