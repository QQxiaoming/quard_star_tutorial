TARGET = qtvirtualkeyboard_hunspell
QT += qml virtualkeyboard hunspellinputmethod-private

HEADERS += \
    hunspellplugin.h
SOURCES += \
    hunspellplugin.cpp
OTHER_FILES += \
    hunspell.json

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

include(../../../config.pri)

hunspell-library {
    exists(../3rdparty/hunspell/data) {
        hunspell_data.files = \
            $$PWD/../3rdparty/hunspell/data/*.dic \
            $$PWD/../3rdparty/hunspell/data/*.aff
        hunspell_data.path = $$VIRTUALKEYBOARD_INSTALL_DATA/hunspell
        INSTALLS += hunspell_data
        !prefix_build: COPIES += hunspell_data
    } else {
        error("Hunspell dictionaries are missing! Please copy .dic and .aff" \
              "files to src/plugins/hunspell/3rdparty/hunspell/data directory.")
    }
}

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard Hunspell (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardHunspellPlugin
load(qt_plugin)
