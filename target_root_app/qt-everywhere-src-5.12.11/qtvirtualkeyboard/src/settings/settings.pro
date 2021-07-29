TARGET = qtquickvirtualkeyboardsettingsplugin
TARGETPATH = QtQuick/VirtualKeyboard/Settings
IMPORT_VERSION = 2.2
QT += qml quick virtualkeyboard-private

CONFIG += no_cxx_module

SOURCES += \
    qtquickvirtualkeyboardsettingsplugin.cpp

HEADERS += \
    qtquickvirtualkeyboardsettingsplugin.h

OTHER_FILES += \
    plugins.qmltypes \
    qmldir

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard for Qt."
}

load(qml_plugin)
