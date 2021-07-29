TARGET = qmldbg_tcp
QT = qml-private network

SOURCES += \
    $$PWD/qtcpserverconnection.cpp

HEADERS += \
    $$PWD/qtcpserverconnectionfactory.h

OTHER_FILES += \
    $$PWD/qtcpserverconnection.json

PLUGIN_TYPE = qmltooling
PLUGIN_CLASS_NAME = QTcpServerConnectionFactory
load(qt_plugin)
