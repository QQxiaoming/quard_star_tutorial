TARGET = qtvirtualkeyboard_pinyin
QT += qml virtualkeyboard

HEADERS += \
    pinyindecoderservice_p.h \
    pinyininputmethod_p.h \
    pinyinplugin.h
SOURCES += \
    pinyindecoderservice.cpp \
    pinyininputmethod.cpp \
    pinyinplugin.cpp
OTHER_FILES += \
    pinyin.json

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

include(../../../config.pri)

LAYOUT_FILES += \
    $$LAYOUTS_BASE/content/layouts/zh_CN/dialpad.fallback \
    $$LAYOUTS_BASE/content/layouts/zh_CN/digits.fallback \
    $$LAYOUTS_BASE/content/layouts/zh_CN/main.qml \
    $$LAYOUTS_BASE/content/layouts/zh_CN/numbers.fallback \
    $$LAYOUTS_BASE/content/layouts/zh_CN/symbols.qml

OTHER_FILES += \
    $$LAYOUT_FILES

virtualkeyboard_pinyin_layouts.files = $$LAYOUT_FILES
virtualkeyboard_pinyin_layouts.base = $$LAYOUTS_BASE
virtualkeyboard_pinyin_layouts.prefix = $$LAYOUTS_PREFIX
RESOURCES += virtualkeyboard_pinyin_layouts

QMAKE_USE += pinyin
!no-bundle-pinyin {
    PINYIN_FILES += ../3rdparty/pinyin/data/dict_pinyin.dat
    pinyin.files = $$PINYIN_FILES
    pinyin.base = $$PWD/..
    pinyin.prefix = /QtQuick/VirtualKeyboard
    RESOURCES += pinyin
} else {
    pinyin_data.files = $$PWD/../3rdparty/pinyin/data/dict_pinyin.dat
    pinyin_data.path = $$DATAPATH/pinyin
    INSTALLS += pinyin_data
    !prefix_build: COPIES += pinyin_data
}

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard Pinyin (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardPinyinPlugin
load(qt_plugin)
