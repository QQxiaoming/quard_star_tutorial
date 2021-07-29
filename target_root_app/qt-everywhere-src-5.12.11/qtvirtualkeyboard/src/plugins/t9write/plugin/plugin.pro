TARGET = qtvirtualkeyboard_t9write
QT += qml virtualkeyboard-private

HEADERS += \
    t9write_p.h \
    t9writedictionary_p.h \
    t9writeinputmethod_p.h \
    t9writeplugin.h \
    t9writeworker_p.h
SOURCES += \
    t9writedictionary.cpp \
    t9writeinputmethod.cpp \
    t9writeplugin.cpp \
    t9writeworker.cpp
OTHER_FILES += \
    t9write.json

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

include(../../../config.pri)
include(../3rdparty/t9write/t9write-build.pri)

equals(T9WRITE_FOUND, 0): \
    error("T9Write SDK could not be found. For more information, see" \
          "the documentation in Building Qt Virtual Keyboard")

LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/fallback/handwriting.qml
t9write-alphabetic {
    contains(CONFIG, lang-en(_GB)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/en_GB/handwriting.fallback
    contains(CONFIG, lang-en(_US)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/en_US/handwriting.fallback
    contains(CONFIG, lang-ar.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ar_AR/handwriting.qml
    contains(CONFIG, lang-bg.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/bg_BG/handwriting.qml
    contains(CONFIG, lang-cs.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/cs_CZ/handwriting.qml
    contains(CONFIG, lang-da.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/da_DK/handwriting.fallback
    contains(CONFIG, lang-de.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/de_DE/handwriting.fallback
    contains(CONFIG, lang-el.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/el_GR/handwriting.qml
    contains(CONFIG, lang-es(_ES)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/es_ES/handwriting.qml
    contains(CONFIG, lang-es(_MX)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/es_MX/handwriting.qml
    contains(CONFIG, lang-et.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/et_EE/handwriting.fallback
    contains(CONFIG, lang-fa.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/fa_FA/handwriting.qml
    contains(CONFIG, lang-fi.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/fi_FI/handwriting.fallback
    contains(CONFIG, lang-fr(_FR)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/fr_FR/handwriting.fallback
    contains(CONFIG, lang-fr(_CA)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/fr_CA/handwriting.fallback
    contains(CONFIG, lang-he.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/he_IL/handwriting.qml
    contains(CONFIG, lang-hr.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/hr_HR/handwriting.qml
    contains(CONFIG, lang-hu.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/hu_HU/handwriting.qml
    contains(CONFIG, lang-id.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/id_ID/handwriting.fallback
    contains(CONFIG, lang-it.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/it_IT/handwriting.fallback
    contains(CONFIG, lang-ms.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ms_MY/handwriting.fallback
    contains(CONFIG, lang-nb.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/nb_NO/handwriting.fallback
    contains(CONFIG, lang-nl.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/nl_NL/handwriting.fallback
    contains(CONFIG, lang-pl.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/pl_PL/handwriting.fallback
    contains(CONFIG, lang-pt(_BR)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/pt_BR/handwriting.fallback
    contains(CONFIG, lang-pt(_PT)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/pt_PT/handwriting.fallback
    contains(CONFIG, lang-ro.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ro_RO/handwriting.qml
    contains(CONFIG, lang-ru.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ru_RU/handwriting.fallback
    contains(CONFIG, lang-sk.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/sk_SK/handwriting.fallback
    contains(CONFIG, lang-sl.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/sl_SI/handwriting.fallback
    contains(CONFIG, lang-sq.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/sq_AL/handwriting.fallback
    contains(CONFIG, lang-sr.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/sr_SP/handwriting.qml
    contains(CONFIG, lang-sv.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/sv_SE/handwriting.fallback
    contains(CONFIG, lang-th.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/th_TH/handwriting.qml
    contains(CONFIG, lang-tr.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/tr_TR/handwriting.fallback
    contains(CONFIG, lang-uk.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/uk_UA/handwriting.fallback
    contains(CONFIG, lang-vi.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/vi_VN/handwriting.fallback
}
t9write-cjk {
    contains(CONFIG, lang-ja.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ja_JP/handwriting.qml
    contains(CONFIG, lang-ko.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ko_KR/handwriting.qml
    contains(CONFIG, lang-zh(_CN)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/zh_CN/handwriting.qml
    contains(CONFIG, lang-zh(_TW)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/zh_TW/handwriting.qml
}

OTHER_FILES += \
    $$LAYOUT_FILES

virtualkeyboard_t9write_layouts.files = $$LAYOUT_FILES
virtualkeyboard_t9write_layouts.base = $$LAYOUTS_BASE
virtualkeyboard_t9write_layouts.prefix = $$LAYOUTS_PREFIX
RESOURCES += virtualkeyboard_t9write_layouts

DEFINES += HAVE_T9WRITE
QMAKE_USE += t9write_db
INCLUDEPATH += $$T9WRITE_INCLUDE_DIRS
t9write-alphabetic {
    LIBS += $$T9WRITE_ALPHABETIC_LIBS
    DEFINES += HAVE_T9WRITE_ALPHABETIC
    !isEmpty(T9WRITE_ALPHABETIC_BINS) {
        t9write_alphabetic_bins.files = $$T9WRITE_ALPHABETIC_BINS
        t9write_alphabetic_bins.path = $$[QT_INSTALL_BINS]
        INSTALLS += t9write_alphabetic_bins
        !prefix_build: COPIES += t9write_alphabetic_bins
    }
}
t9write-cjk {
    LIBS += $$T9WRITE_CJK_LIBS
    DEFINES += HAVE_T9WRITE_CJK
    !isEmpty(T9WRITE_CJK_BINS) {
        t9write_cjk_bins.files = $$T9WRITE_CJK_BINS
        t9write_cjk_bins.path = $$[QT_INSTALL_BINS]
        INSTALLS += t9write_cjk_bins
        !prefix_build: COPIES += t9write_cjk_bins
    }
}

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard T9 Write (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardT9WritePlugin
load(qt_plugin)
