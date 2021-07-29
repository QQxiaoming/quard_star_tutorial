CXX_MODULE = qml
TARGET  = qmlsettingsplugin
TARGETPATH = Qt/labs/settings
IMPORT_VERSION = 1.1

QT = core qml

HEADERS += \
    qqmlsettings_p.h

SOURCES += \
    plugin.cpp \
    qqmlsettings.cpp

load(qml_plugin)
