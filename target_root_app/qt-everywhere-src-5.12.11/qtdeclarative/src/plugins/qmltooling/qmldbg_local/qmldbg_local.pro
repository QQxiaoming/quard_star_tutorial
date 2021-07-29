TARGET = qmldbg_local
QT = qml-private

SOURCES += \
    $$PWD/qlocalclientconnection.cpp

HEADERS += \
    $$PWD/qlocalclientconnectionfactory.h

OTHER_FILES += \
    $$PWD/qlocalclientconnection.json

PLUGIN_TYPE = qmltooling
PLUGIN_CLASS_NAME = QLocalClientConnectionFactory
load(qt_plugin)
