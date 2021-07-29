CXX_MODULE = qml
TARGET = qtwebengineplugin
TARGETPATH = QtWebEngine
IMPORT_VERSION = 1.8

QT += webengine qml quick
QT_PRIVATE += core-private webenginecore-private webengine-private

INCLUDEPATH += $$QTWEBENGINE_ROOT/src/core $$QTWEBENGINE_ROOT/src/core/api $$QTWEBENGINE_ROOT/src/webengine $$QTWEBENGINE_ROOT/src/webengine/api $$QTWEBENGINE_ROOT/include/QtWebEngine

SOURCES = plugin.cpp

load(qml_plugin)
