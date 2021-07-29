CXX_MODULE = qml
TARGET = qtremoteobjects
TARGETPATH = QtRemoteObjects
IMPORT_VERSION = 5.12

QT += qml remoteobjects

SOURCES = \
    $$PWD/plugin.cpp \

HEADERS = \

load(qml_plugin)
