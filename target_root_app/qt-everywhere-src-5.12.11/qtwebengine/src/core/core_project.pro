TEMPLATE = lib
# Fake project to make QtCreator happy.

include(core_common.pri)

linking_pri = $$OUT_PWD/$$getConfigDir()/$${TARGET}.pri

!include($$linking_pri) {
    error("Could not find the linking information that gn should have generated.")
}

CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
INCLUDEPATH += $$CHROMIUM_SRC_DIR \
               $$CHROMIUM_SRC_DIR/third_party/blink/public \
               $$OUT_PWD/$$getConfigDir()/gen

SOURCES += $$NINJA_SOURCES
HEADERS += $$NINJA_HEADERS
DEFINES += $$NINJA_DEFINES

lupdate_run {
    SOURCES += clipboard_qt.cpp \
        profile_adapter_client.cpp \
        profile_adapter.cpp \
        render_view_context_menu_qt.cpp \
        web_contents_adapter.cpp
}
