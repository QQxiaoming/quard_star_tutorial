TEMPLATE = app
QT = qml-private core-private
CONFIG += no_import_scan
SOURCES = qmljs.cpp

include($$PWD/../../src/3rdparty/masm/masm-defs.pri)

QMAKE_TARGET_DESCRIPTION = QML JavaScript Tool

load(qt_tool)
