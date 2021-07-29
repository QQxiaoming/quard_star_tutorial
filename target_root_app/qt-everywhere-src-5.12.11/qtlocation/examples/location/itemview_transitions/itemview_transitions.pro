TEMPLATE = app

QT += qml quick location

CONFIG += c++11

SOURCES += main.cpp

qmlfiles.files = $$files($$PWD/*.qml)
qmlfiles.prefix = /
qmlfiles.base = $$PWD

RESOURCES += qmlfiles

DISTFILES += $$files($$PWD/*.py) $$files($$PWD/*.GeoJSON) main.qml OsloListModel.qml doc/src/itemview_transitions.qdoc

target.path = $$[QT_INSTALL_EXAMPLES]/location/itemview_transitions
INSTALLS += target
