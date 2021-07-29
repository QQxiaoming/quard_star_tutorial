TARGET = qtvirtualkeyboard_hangul
QT += qml virtualkeyboard

HEADERS += \
    hangul_p.h \
    hangulplugin.h \
    hangulinputmethod_p.h
SOURCES += \
    hangul.cpp \
    hangulplugin.cpp \
    hangulinputmethod.cpp
OTHER_FILES += \
    hangul.json

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

include(../../config.pri)

LAYOUT_FILES += \
    $$LAYOUTS_BASE/content/layouts/ko_KR/dialpad.fallback \
    $$LAYOUTS_BASE/content/layouts/ko_KR/digits.fallback \
    $$LAYOUTS_BASE/content/layouts/ko_KR/main.qml \
    $$LAYOUTS_BASE/content/layouts/ko_KR/numbers.fallback \
    $$LAYOUTS_BASE/content/layouts/ko_KR/symbols.qml

OTHER_FILES += \
    $$LAYOUT_FILES

virtualkeyboard_hangul_layouts.files = $$LAYOUT_FILES
virtualkeyboard_hangul_layouts.base = $$LAYOUTS_BASE
virtualkeyboard_hangul_layouts.prefix = $$LAYOUTS_PREFIX
RESOURCES += virtualkeyboard_hangul_layouts

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard Hangul (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardHangulPlugin
load(qt_plugin)
