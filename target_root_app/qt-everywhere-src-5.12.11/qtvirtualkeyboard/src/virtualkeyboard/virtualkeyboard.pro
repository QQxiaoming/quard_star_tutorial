TARGET  = QtVirtualKeyboard
MODULE = virtualkeyboard
MODULE_PLUGIN_TYPES = virtualkeyboard

QMAKE_DOCS = $$PWD/doc/qtvirtualkeyboard.qdocconf
include(doc/doc.pri)

QT += qml quick gui gui-private core-private
CONFIG += qtquickcompiler

DEFINES += QVIRTUALKEYBOARD_LIBRARY

include(../config.pri)

SOURCES += \
    platforminputcontext.cpp \
    qvirtualkeyboardinputcontext.cpp \
    qvirtualkeyboardinputcontext_p.cpp \
    qvirtualkeyboardabstractinputmethod.cpp \
    plaininputmethod.cpp \
    qvirtualkeyboardinputengine.cpp \
    shifthandler.cpp \
    inputmethod.cpp \
    inputselectionhandle.cpp \
    qvirtualkeyboardselectionlistmodel.cpp \
    fallbackinputmethod.cpp \
    abstractinputpanel.cpp \
    appinputpanel.cpp \
    enterkeyaction.cpp \
    enterkeyactionattachedtype.cpp \
    settings.cpp \
    virtualkeyboardsettings.cpp \
    qvirtualkeyboardtrace.cpp \
    desktopinputselectioncontrol.cpp \
    shadowinputcontext.cpp \
    gesturerecognizer.cpp \
    handwritinggesturerecognizer.cpp \
    qvirtualkeyboardextensionplugin.cpp

HEADERS += \
    platforminputcontext_p.h \
    qvirtualkeyboardinputcontext.h \
    qvirtualkeyboardinputcontext_p.h \
    qvirtualkeyboardabstractinputmethod.h \
    plaininputmethod_p.h \
    qvirtualkeyboardinputengine.h \
    shifthandler_p.h \
    inputmethod_p.h \
    inputselectionhandle_p.h \
    qvirtualkeyboardselectionlistmodel.h \
    fallbackinputmethod_p.h \
    abstractinputpanel_p.h \
    appinputpanel_p.h \
    appinputpanel_p_p.h \
    virtualkeyboarddebug_p.h \
    enterkeyaction_p.h \
    enterkeyactionattachedtype_p.h \
    settings_p.h \
    virtualkeyboardsettings_p.h \
    qvirtualkeyboardtrace.h \
    desktopinputselectioncontrol_p.h \
    shadowinputcontext_p.h \
    gesturerecognizer_p.h \
    handwritinggesturerecognizer_p.h \
    qvirtualkeyboard_global.h \
    qvirtualkeyboardextensionplugin.h \
    qvirtualkeyboard_staticplugin_p.h

!no-builtin-style: RESOURCES += \
    content/styles/default/virtualkeyboard_default_style.qrc \
    content/styles/retro/virtualkeyboard_retro_style.qrc

RESOURCES += \
    content/virtualkeyboard_content.qrc

# Fallback for languages which don't have these special layouts
LAYOUT_FILES += \
    content/layouts/fallback/dialpad.qml \
    content/layouts/fallback/digits.qml \
    content/layouts/fallback/numbers.qml \
    content/layouts/fallback/main.qml \
    content/layouts/fallback/symbols.qml
contains(CONFIG, lang-en(_GB)?) {
    LAYOUT_FILES += \
        content/layouts/en_GB/dialpad.fallback \
        content/layouts/en_GB/digits.fallback \
        content/layouts/en_GB/main.fallback \
        content/layouts/en_GB/numbers.fallback \
        content/layouts/en_GB/symbols.fallback
}
contains(CONFIG, lang-en(_US)?) {
    LAYOUT_FILES += \
        content/layouts/en_US/dialpad.fallback \
        content/layouts/en_US/digits.fallback \
        content/layouts/en_US/main.fallback \
        content/layouts/en_US/numbers.fallback \
        content/layouts/en_US/symbols.fallback
}
contains(CONFIG, lang-ar.*) {
    LAYOUT_FILES += \
        content/layouts/ar_AR/dialpad.fallback \
        content/layouts/ar_AR/digits.qml \
        content/layouts/ar_AR/main.qml \
        content/layouts/ar_AR/numbers.qml \
        content/layouts/ar_AR/symbols.qml
}
contains(CONFIG, lang-bg.*) {
    LAYOUT_FILES += \
        content/layouts/bg_BG/dialpad.fallback \
        content/layouts/bg_BG/digits.fallback \
        content/layouts/bg_BG/main.qml \
        content/layouts/bg_BG/numbers.fallback \
        content/layouts/bg_BG/symbols.fallback
}
contains(CONFIG, lang-cs.*) {
    LAYOUT_FILES += \
        content/layouts/cs_CZ/dialpad.fallback \
        content/layouts/cs_CZ/digits.fallback \
        content/layouts/cs_CZ/main.qml \
        content/layouts/cs_CZ/numbers.fallback \
        content/layouts/cs_CZ/symbols.fallback
}
contains(CONFIG, lang-da.*) {
    LAYOUT_FILES += \
        content/layouts/da_DK/dialpad.fallback \
        content/layouts/da_DK/digits.fallback \
        content/layouts/da_DK/main.qml \
        content/layouts/da_DK/numbers.fallback \
        content/layouts/da_DK/symbols.fallback
}
contains(CONFIG, lang-de.*) {
    LAYOUT_FILES += \
        content/layouts/de_DE/dialpad.fallback \
        content/layouts/de_DE/digits.fallback \
        content/layouts/de_DE/main.qml \
        content/layouts/de_DE/numbers.fallback \
        content/layouts/de_DE/symbols.fallback
}
contains(CONFIG, lang-el.*) {
    LAYOUT_FILES += \
        content/layouts/el_GR/dialpad.fallback \
        content/layouts/el_GR/digits.fallback \
        content/layouts/el_GR/main.qml \
        content/layouts/el_GR/numbers.fallback \
        content/layouts/el_GR/symbols.fallback
}
contains(CONFIG, lang-es(_ES)?) {
    LAYOUT_FILES += \
        content/layouts/es_ES/dialpad.fallback \
        content/layouts/es_ES/digits.fallback \
        content/layouts/es_ES/main.qml \
        content/layouts/es_ES/numbers.fallback \
        content/layouts/es_ES/symbols.qml
}
contains(CONFIG, lang-es(_MX)?) {
    LAYOUT_FILES += \
        content/layouts/es_MX/dialpad.fallback \
        content/layouts/es_MX/digits.fallback \
        content/layouts/es_MX/main.qml \
        content/layouts/es_MX/numbers.fallback \
        content/layouts/es_MX/symbols.qml
}
contains(CONFIG, lang-et.*) {
    LAYOUT_FILES += \
        content/layouts/et_EE/dialpad.fallback \
        content/layouts/et_EE/digits.fallback \
        content/layouts/et_EE/main.qml \
        content/layouts/et_EE/numbers.fallback \
        content/layouts/et_EE/symbols.fallback
}
contains(CONFIG, lang-fa.*) {
    LAYOUT_FILES += \
        content/layouts/fa_FA/dialpad.fallback \
        content/layouts/fa_FA/digits.qml \
        content/layouts/fa_FA/main.qml \
        content/layouts/fa_FA/numbers.qml \
        content/layouts/fa_FA/symbols.qml
}
contains(CONFIG, lang-fi.*) {
    LAYOUT_FILES += \
        content/layouts/fi_FI/dialpad.fallback \
        content/layouts/fi_FI/digits.fallback \
        content/layouts/fi_FI/main.qml \
        content/layouts/fi_FI/numbers.fallback \
        content/layouts/fi_FI/symbols.fallback
}
contains(CONFIG, lang-fr(_CA)?) {
    LAYOUT_FILES += \
        content/layouts/fr_CA/dialpad.fallback \
        content/layouts/fr_CA/digits.fallback \
        content/layouts/fr_CA/main.qml \
        content/layouts/fr_CA/numbers.fallback \
        content/layouts/fr_CA/symbols.fallback
}
contains(CONFIG, lang-fr(_FR)?) {
    LAYOUT_FILES += \
        content/layouts/fr_FR/dialpad.fallback \
        content/layouts/fr_FR/digits.fallback \
        content/layouts/fr_FR/main.qml \
        content/layouts/fr_FR/numbers.fallback \
        content/layouts/fr_FR/symbols.fallback
}
contains(CONFIG, lang-he.*) {
    LAYOUT_FILES += \
        content/layouts/he_IL/dialpad.fallback \
        content/layouts/he_IL/digits.fallback \
        content/layouts/he_IL/main.qml \
        content/layouts/he_IL/numbers.fallback \
        content/layouts/he_IL/symbols.qml
}
contains(CONFIG, lang-hi.*) {
    LAYOUT_FILES += \
        content/layouts/hi_IN/dialpad.fallback \
        content/layouts/hi_IN/digits.fallback \
        content/layouts/hi_IN/main.qml \
        content/layouts/hi_IN/numbers.fallback \
        content/layouts/hi_IN/symbols.qml
}
contains(CONFIG, lang-hr.*) {
    LAYOUT_FILES += \
        content/layouts/hr_HR/dialpad.fallback \
        content/layouts/hr_HR/digits.fallback \
        content/layouts/hr_HR/main.qml \
        content/layouts/hr_HR/numbers.fallback \
        content/layouts/hr_HR/symbols.fallback
}
contains(CONFIG, lang-hu.*) {
    LAYOUT_FILES += \
        content/layouts/hu_HU/dialpad.fallback \
        content/layouts/hu_HU/digits.fallback \
        content/layouts/hu_HU/main.qml \
        content/layouts/hu_HU/numbers.fallback \
        content/layouts/hu_HU/symbols.fallback
}
contains(CONFIG, lang-id.*) {
    LAYOUT_FILES += \
        content/layouts/id_ID/dialpad.fallback \
        content/layouts/id_ID/digits.fallback \
        content/layouts/id_ID/main.fallback \
        content/layouts/id_ID/numbers.fallback \
        content/layouts/id_ID/symbols.fallback
}
contains(CONFIG, lang-it.*) {
    LAYOUT_FILES += \
        content/layouts/it_IT/dialpad.fallback \
        content/layouts/it_IT/digits.fallback \
        content/layouts/it_IT/main.qml \
        content/layouts/it_IT/numbers.fallback \
        content/layouts/it_IT/symbols.fallback
}
contains(CONFIG, lang-nb.*) {
    LAYOUT_FILES += \
        content/layouts/nb_NO/dialpad.fallback \
        content/layouts/nb_NO/digits.fallback \
        content/layouts/nb_NO/main.qml \
        content/layouts/nb_NO/numbers.fallback \
        content/layouts/nb_NO/symbols.fallback
}
contains(CONFIG, lang-ms.*) {
    LAYOUT_FILES += \
        content/layouts/ms_MY/dialpad.fallback \
        content/layouts/ms_MY/digits.fallback \
        content/layouts/ms_MY/main.fallback \
        content/layouts/ms_MY/numbers.fallback \
        content/layouts/ms_MY/symbols.fallback
}
contains(CONFIG, lang-nl.*) {
    LAYOUT_FILES += \
        content/layouts/nl_NL/dialpad.fallback \
        content/layouts/nl_NL/digits.fallback \
        content/layouts/nl_NL/main.fallback \
        content/layouts/nl_NL/numbers.fallback \
        content/layouts/nl_NL/symbols.fallback
}
contains(CONFIG, lang-pl.*) {
    LAYOUT_FILES += \
        content/layouts/pl_PL/dialpad.fallback \
        content/layouts/pl_PL/digits.fallback \
        content/layouts/pl_PL/main.qml \
        content/layouts/pl_PL/numbers.fallback \
        content/layouts/pl_PL/symbols.fallback
}
contains(CONFIG, lang-pt(_BR)?) {
    LAYOUT_FILES += \
        content/layouts/pt_BR/dialpad.fallback \
        content/layouts/pt_BR/digits.fallback \
        content/layouts/pt_BR/main.qml \
        content/layouts/pt_BR/numbers.fallback \
        content/layouts/pt_BR/symbols.fallback
}
contains(CONFIG, lang-pt(_PT)?) {
    LAYOUT_FILES += \
        content/layouts/pt_PT/dialpad.fallback \
        content/layouts/pt_PT/digits.fallback \
        content/layouts/pt_PT/main.qml \
        content/layouts/pt_PT/numbers.fallback \
        content/layouts/pt_PT/symbols.fallback
}
contains(CONFIG, lang-ro.*) {
    LAYOUT_FILES += \
        content/layouts/ro_RO/dialpad.fallback \
        content/layouts/ro_RO/digits.fallback \
        content/layouts/ro_RO/main.qml \
        content/layouts/ro_RO/numbers.fallback \
        content/layouts/ro_RO/symbols.fallback
}
contains(CONFIG, lang-ru.*) {
    LAYOUT_FILES += \
        content/layouts/ru_RU/dialpad.fallback \
        content/layouts/ru_RU/digits.fallback \
        content/layouts/ru_RU/main.qml \
        content/layouts/ru_RU/numbers.fallback \
        content/layouts/ru_RU/symbols.fallback
}
contains(CONFIG, lang-sk.*) {
    LAYOUT_FILES += \
        content/layouts/sk_SK/dialpad.fallback \
        content/layouts/sk_SK/digits.fallback \
        content/layouts/sk_SK/main.qml \
        content/layouts/sk_SK/numbers.fallback \
        content/layouts/sk_SK/symbols.fallback
}
contains(CONFIG, lang-sl.*) {
    LAYOUT_FILES += \
        content/layouts/sl_SI/dialpad.fallback \
        content/layouts/sl_SI/digits.fallback \
        content/layouts/sl_SI/main.qml \
        content/layouts/sl_SI/numbers.fallback \
        content/layouts/sl_SI/symbols.fallback
}
contains(CONFIG, lang-sq.*) {
    LAYOUT_FILES += \
        content/layouts/sq_AL/dialpad.fallback \
        content/layouts/sq_AL/digits.fallback \
        content/layouts/sq_AL/main.qml \
        content/layouts/sq_AL/numbers.fallback \
        content/layouts/sq_AL/symbols.fallback
}
contains(CONFIG, lang-sr.*) {
    LAYOUT_FILES += \
        content/layouts/sr_SP/dialpad.fallback \
        content/layouts/sr_SP/digits.fallback \
        content/layouts/sr_SP/main.qml \
        content/layouts/sr_SP/numbers.fallback \
        content/layouts/sr_SP/symbols.fallback
}
contains(CONFIG, lang-sv.*) {
    LAYOUT_FILES += \
        content/layouts/sv_SE/dialpad.fallback \
        content/layouts/sv_SE/digits.fallback \
        content/layouts/sv_SE/main.qml \
        content/layouts/sv_SE/numbers.fallback \
        content/layouts/sv_SE/symbols.fallback
}
contains(CONFIG, lang-tr.*) {
    LAYOUT_FILES += \
        content/layouts/tr_TR/dialpad.fallback \
        content/layouts/tr_TR/digits.fallback \
        content/layouts/tr_TR/main.qml \
        content/layouts/tr_TR/numbers.fallback \
        content/layouts/tr_TR/symbols.fallback
}
contains(CONFIG, lang-uk.*) {
    LAYOUT_FILES += \
        content/layouts/uk_UA/dialpad.fallback \
        content/layouts/uk_UA/digits.fallback \
        content/layouts/uk_UA/main.qml \
        content/layouts/uk_UA/numbers.fallback \
        content/layouts/uk_UA/symbols.fallback
}
contains(CONFIG, lang-vi.*) {
    LAYOUT_FILES += \
        content/layouts/vi_VN/dialpad.fallback \
        content/layouts/vi_VN/digits.fallback \
        content/layouts/vi_VN/main.qml \
        content/layouts/vi_VN/numbers.fallback \
        content/layouts/vi_VN/symbols.qml
}

no-builtin-style {
    DEFINES += QT_VIRTUALKEYBOARD_DEFAULT_STYLE=\\\"\\\"
} else:retro-style {
    DEFINES += QT_VIRTUALKEYBOARD_DEFAULT_STYLE=\\\"retro\\\"
} else {
    DEFINES += QT_VIRTUALKEYBOARD_DEFAULT_STYLE=\\\"default\\\"
}

DEFINES += QT_VIRTUALKEYBOARD_DEFAULT_LAYOUTS_DIR=\\\"qrc:/QtQuick/VirtualKeyboard/content/layouts\\\"

DEFINES += \
    QT_NO_CAST_TO_ASCII \
    QT_ASCII_CAST_WARNINGS \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_FROM_BYTEARRAY

OTHER_FILES += \
    content/styles/default/*.qml \
    content/styles/retro/*.qml \
    content/*.qml \
    content/components/*.qml \
    qtvirtualkeyboard.json \
    $$LAYOUT_FILES

!disable-desktop:isEmpty(CROSS_COMPILE):!android-embedded:!qnx {
    SOURCES += desktopinputpanel.cpp inputview.cpp
    HEADERS += desktopinputpanel_p.h inputview_p.h
    DEFINES += QT_VIRTUALKEYBOARD_DESKTOP
    !no-pkg-config:packagesExist(xcb) {
        PKGCONFIG += xcb xcb-xfixes
        DEFINES += QT_VIRTUALKEYBOARD_HAVE_XCB
    }
}

record-trace-input {
    SOURCES += unipentrace.cpp
    HEADERS += unipentrace_p.h
    MODULE_DEFINES += QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
}

arrow-key-navigation: DEFINES += QT_VIRTUALKEYBOARD_ARROW_KEY_NAVIGATION

!disable-layouts {
    virtualkeyboard_layouts.files = $$LAYOUT_FILES
    virtualkeyboard_layouts.prefix = $$LAYOUTS_PREFIX
    RESOURCES += virtualkeyboard_layouts
    DEFINES += HAVE_LAYOUTS
}

load(qt_module)
