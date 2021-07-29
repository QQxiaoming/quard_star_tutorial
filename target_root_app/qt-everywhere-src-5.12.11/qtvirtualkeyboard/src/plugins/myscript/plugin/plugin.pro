TARGET = qtvirtualkeyboard_myscript
QT += qml virtualkeyboard

include(../../../config.pri)

equals(MYSCRIPT_FOUND, 0): \
    error("MyScript SDK could not be found. For more information, see" \
          "the documentation in Building Qt Virtual Keyboard")
HEADERS += \
    myscriptplugin.h \
    myscriptinputmethod_p.h \
    myscriptinputmethod_p_p.h
SOURCES += \
    myscriptplugin.cpp \
    myscriptinputmethod.cpp \
    $$MYSCRIPT_PATH/voim/api/c/examples/common/Properties.c \
    $$MYSCRIPT_PATH/voim/api/c/examples/common/PortabilityDefinitions.c
OTHER_FILES += \
    myscript.json
INCLUDEPATH += \
    $$MYSCRIPT_PATH/voim/api/c/include \
    $$MYSCRIPT_PATH/voim/api/c/examples
MYSCRIPT_DATA = qtvirtualkeyboard/myscript
DEFINES += MYSCRIPT_CERTIFICATE=\\\"$$MYSCRIPT_PATH/edk/c/examples/certificates/MyCertificate.c\\\"
DEFINES += MYSCRIPT_VOIM_PROPERTY_PATH=\\\"$$MYSCRIPT_DATA/$$MYSCRIPT_VOIM_CONF\\\"
DEFINES += MYSCRIPT_LANGUAGE_CONF_PATH=\\\"$$MYSCRIPT_DATA/$$MYSCRIPT_LANGUAGE_CONF\\\"
DEFINES += MYSCRIPT_VOIM_NAME=\\\"$${MYSCRIPT_LIB_PREFIX}voim$${MYSCRIPT_LIB_SUFFIX}\\\"
DEFINES += MYSCRIPT_ENGINE_NAME=\\\"$${MYSCRIPT_LIB_PREFIX}MyScriptEngine$${MYSCRIPT_LIB_SUFFIX}\\\"
LIBS += $$MYSCRIPT_VOIM_LIB
unix:linux:!android: QMAKE_RPATHDIR += $$MYSCRIPT_PATH/$$MYSCRIPT_VOIM_PATH
myscript_engine_bins.files = $$MYSCRIPT_ENGINE_BINS
myscript_engine_bins.path = $$[QT_INSTALL_BINS]
myscript_voim_conf.files = $$MYSCRIPT_PATH/$$MYSCRIPT_VOIM_CONF
myscript_voim_conf.path = $$[QT_INSTALL_DATA]/$$MYSCRIPT_DATA/voim
myscript_language_conf.files = $$MYSCRIPT_PATH/$$MYSCRIPT_LANGUAGE_CONF
myscript_language_conf.path = $$[QT_INSTALL_DATA]/$$MYSCRIPT_DATA
myscript_resources.files = $$MYSCRIPT_PATH/$$MYSCRIPT_RESOURCES
myscript_resources.path = $$[QT_INSTALL_DATA]/$$MYSCRIPT_DATA
INSTALLS += \
    myscript_engine_bins \
    myscript_voim_conf \
    myscript_language_conf \
    myscript_resources
!prefix_build: COPIES += \
    myscript_engine_bins \
    myscript_voim_conf \
    myscript_language_conf \
    myscript_resources

LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/fallback/handwriting.qml
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
contains(CONFIG, lang-hi.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/hi_IN/handwriting.qml
contains(CONFIG, lang-hr.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/hr_HR/handwriting.qml
contains(CONFIG, lang-hu.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/hu_HU/handwriting.qml
contains(CONFIG, lang-id.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/id_ID/handwriting.fallback
contains(CONFIG, lang-it.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/it_IT/handwriting.fallback
contains(CONFIG, lang-ja.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ja_JP/handwriting.qml
contains(CONFIG, lang-ko.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/ko_KR/handwriting.qml
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
contains(CONFIG, lang-th.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/th_TH/handwriting.fallback
contains(CONFIG, lang-tr.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/tr_TR/handwriting.fallback
contains(CONFIG, lang-uk.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/uk_UA/handwriting.fallback
contains(CONFIG, lang-vi.*): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/vi_VN/handwriting.fallback
contains(CONFIG, lang-zh(_CN)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/zh_CN/handwriting.qml
contains(CONFIG, lang-zh(_TW)?): LAYOUT_FILES += $$LAYOUTS_BASE/content/layouts/zh_TW/handwriting.qml

OTHER_FILES += \
    $$LAYOUT_FILES

virtualkeyboard_myscript_layouts.files = $$LAYOUT_FILES
virtualkeyboard_myscript_layouts.base = $$LAYOUTS_BASE
virtualkeyboard_myscript_layouts.prefix = $$LAYOUTS_PREFIX
RESOURCES += virtualkeyboard_myscript_layouts

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard MyScript (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard Extension for Qt."
}

PLUGIN_TYPE = virtualkeyboard
PLUGIN_CLASS_NAME = QtVirtualKeyboardMyScriptPlugin
load(qt_plugin)
