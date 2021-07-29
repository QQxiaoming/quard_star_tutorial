CXX_MODULE = qml
TARGET  = qmlfolderlistmodelplugin
TARGETPATH = Qt/labs/folderlistmodel
IMPORT_VERSION = 2.$$QT_MINOR_VERSION

QT = core-private qml-private

SOURCES += qquickfolderlistmodel.cpp plugin.cpp \
    fileinfothread.cpp
HEADERS += qquickfolderlistmodel.h \
    fileproperty_p.h \
    fileinfothread_p.h

load(qml_plugin)
