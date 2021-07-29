QT += quick-private network positioning-private location-private qml-private core-private gui-private

TARGET = locationlabsplugin
CXX_MODULE = $$TARGET
TARGETPATH = Qt/labs/location
IMPORT_VERSION = 1.0

SOURCES += \
           locationlabs.cpp

#CONFIG += no_cxx_module
load(qml_plugin)

OTHER_FILES += \
    plugin.json \
    qmldir
