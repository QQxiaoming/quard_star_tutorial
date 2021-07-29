TARGET = qtvirtualkeyboard_openwnn
QT += qml virtualkeyboard

HEADERS += \
    openwnnplugin.h \
    openwnninputmethod_p.h
SOURCES += \
    openwnnplugin.cpp \
    openwnninputmethod.cpp
OTHER_FILES += \
    openwnn.json

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

include(../../../config.pri)

LAYOUT_FILES += \
    $$LAYOUTS_BASE/content/layouts/ja_JP/dialpad.fallback \
    $$LAYOUTS_BASE/content/layouts/ja_JP/digits.fallback \
    $$LAYOUTS_BASE/content/layouts/ja_JP/main.qml \
    $$LAYOUTS_BASE/content/layouts/ja_JP/numbers.fallback \
    $$LAYOUTS_BASE/content/layouts/ja_JP/symbols.qml

OTHER_FILES += \
    $$LAYOUT_FILES

virtualkeyboard_openwnn_layouts.files = $$LAYOUT_FILES
virtualkeyboard_openwnn_layouts.base = $$LAYOUTS_BASE
virtualkeyboard_openwnn_layouts.prefix = $$LAYOUTS_PREFIX
RESOURCES += virtualkeyboard_openwnn_layouts

QMAKE_USE += openwnn

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard OpenWNN (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardOpenWnnPlugin
load(qt_plugin)
