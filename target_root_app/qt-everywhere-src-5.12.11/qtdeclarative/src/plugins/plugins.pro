TEMPLATE = subdirs
QT_FOR_CONFIG += qml

qtConfig(thread):qtConfig(qml-debug):SUBDIRS += qmltooling
qtHaveModule(quick):SUBDIRS += scenegraph
