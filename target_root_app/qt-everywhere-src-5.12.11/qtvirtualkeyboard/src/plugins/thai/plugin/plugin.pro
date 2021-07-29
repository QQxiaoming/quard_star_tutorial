TARGET = qtvirtualkeyboard_thai
QT += qml virtualkeyboard

HEADERS += \
    thaiinputmethod_p.h \
    thaiplugin.h
SOURCES += \
    thaiinputmethod.cpp \
    thaiplugin.cpp
OTHER_FILES += \
    thai.json

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

include(../../../config.pri)

!disable-hunspell {
    QT += hunspellinputmethod-private
}

LAYOUT_FILES += \
    $$LAYOUTS_BASE/content/layouts/th_TH/dialpad.fallback \
    $$LAYOUTS_BASE/content/layouts/th_TH/digits.fallback \
    $$LAYOUTS_BASE/content/layouts/th_TH/main.qml \
    $$LAYOUTS_BASE/content/layouts/th_TH/numbers.fallback \
    $$LAYOUTS_BASE/content/layouts/th_TH/symbols.qml

OTHER_FILES += \
    $$LAYOUT_FILES

virtualkeyboard_thai_layouts.files = $$LAYOUT_FILES
virtualkeyboard_thai_layouts.base = $$LAYOUTS_BASE
virtualkeyboard_thai_layouts.prefix = $$LAYOUTS_PREFIX
RESOURCES += virtualkeyboard_thai_layouts

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard Thai (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardThaiPlugin
load(qt_plugin)
