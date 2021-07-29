TEMPLATE = app
TARGET = tst_sensors2qmlapi

CONFIG += testcase
QT = core testlib sensors-private qml

SOURCES += tst_sensors2qmlapi.cpp \
           ./../../../src/imports/sensors/qmlsensor.cpp \
           ./../../../src/imports/sensors/qmlsensorgesture.cpp \
           ./../../../src/imports/sensors/qmlsensorrange.cpp \
           qtemplategestureplugin.cpp \
           qtemplaterecognizer.cpp

HEADERS += \
           ./../../../src/imports/sensors/qmlsensor.h \
           ./../../../src/imports/sensors/qmlsensorgesture.h \
           ./../../../src/imports/sensors/qmlsensorrange.h \
           qtemplategestureplugin.h \
           qtemplaterecognizer.h

