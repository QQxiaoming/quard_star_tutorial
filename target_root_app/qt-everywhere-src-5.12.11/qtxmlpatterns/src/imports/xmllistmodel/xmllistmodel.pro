CXX_MODULE = qml
TARGET  = qmlxmllistmodelplugin
TARGETPATH = QtQuick/XmlListModel
IMPORT_VERSION = 2.$$QT_MINOR_VERSION

QT = xmlpatterns qml-private  core-private
qtConfig(qml-network): QT += network

SOURCES += qqmlxmllistmodel.cpp plugin.cpp
HEADERS += qqmlxmllistmodel_p.h

load(qml_plugin)

requires(qtConfig(qml-network))
