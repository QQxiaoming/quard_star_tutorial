win32:{
    PLATFORM_DIR=$$PWD/platform/windows
}

unix:!macx:{
    PLATFORM_DIR=$$PWD/platform/linux
}

macx:{
    PLATFORM_DIR=$$PWD/platform/macos
}

INCLUDEPATH += \
        -I $$PLATFORM_DIR \
        -I $$PWD 

SOURCES += \
    $$PWD/BlockArray.cpp \
    $$PWD/ColorScheme.cpp \
    $$PWD/Emulation.cpp \
    $$PWD/Filter.cpp \
    $$PWD/History.cpp \
    $$PWD/HistorySearch.cpp \
    $$PWD/KeyboardTranslator.cpp \
    $$PWD/konsole_wcwidth.cpp \
    $$PWD/kprocess.cpp \
    $$PWD/qtermwidget.cpp \
    $$PWD/Screen.cpp \
    $$PWD/ScreenWindow.cpp \
    $$PWD/SearchBar.cpp \
    $$PWD/Session.cpp \
    $$PWD/ShellCommand.cpp \
    $$PWD/TerminalCharacterDecoder.cpp \
    $$PWD/TerminalDisplay.cpp \
    $$PWD/tools.cpp \
    $$PWD/Vt102Emulation.cpp \
    $$PLATFORM_DIR/kpty.cpp \
    $$PLATFORM_DIR/kptydevice.cpp \
    $$PLATFORM_DIR/kptyprocess.cpp \
    $$PLATFORM_DIR/Pty.cpp

HEADERS += \
    $$PWD/BlockArray.h \
    $$PWD/CharacterColor.h \
    $$PWD/Character.h \
    $$PWD/ColorScheme.h \
    $$PWD/ColorTables.h \
    $$PWD/DefaultTranslatorText.h \
    $$PWD/Emulation.h \
    $$PWD/ExtendedDefaultTranslator.h \
    $$PWD/Filter.h \
    $$PWD/History.h \
    $$PWD/HistorySearch.h \
    $$PWD/KeyboardTranslator.h \
    $$PWD/konsole_wcwidth.h \
    $$PWD/kprocess.h \
    $$PLATFORM_DIR/kptydevice.h \
    $$PLATFORM_DIR/kpty.h \
    $$PLATFORM_DIR/kpty_p.h \
    $$PLATFORM_DIR/Pty.h \
    $$PLATFORM_DIR/kptyprocess.h \
    $$PWD/LineFont.h \
    $$PWD/qtermwidget.h \
    $$PWD/qtermwidget_export.h \
    $$PWD/qtermwidget_version.h \
    $$PWD/Screen.h \
    $$PWD/ScreenWindow.h \
    $$PWD/SearchBar.h \
    $$PWD/Session.h \
    $$PWD/ShellCommand.h \
    $$PWD/TerminalCharacterDecoder.h \
    $$PWD/TerminalDisplay.h \
    $$PWD/tools.h \
    $$PWD/Vt102Emulation.h

FORMS += \
    $$PWD/SearchBar.ui
    
TRANSLATIONS += \
    $$PWD/translations/qtermwidget_arn.ts \
    $$PWD/translations/qtermwidget_arn.ts \
    $$PWD/translations/qtermwidget_ast.ts \
    $$PWD/translations/qtermwidget_bg.ts \
    $$PWD/translations/qtermwidget_ca.ts \
    $$PWD/translations/qtermwidget_cs.ts \
    $$PWD/translations/qtermwidget_cy.ts \
    $$PWD/translations/qtermwidget_da.ts \
    $$PWD/translations/qtermwidget_de.ts \
    $$PWD/translations/qtermwidget_el.ts \
    $$PWD/translations/qtermwidget_es.ts \
    $$PWD/translations/qtermwidget_et.ts \
    $$PWD/translations/qtermwidget_fr.ts \
    $$PWD/translations/qtermwidget_gl.ts \
    $$PWD/translations/qtermwidget_he.ts \
    $$PWD/translations/qtermwidget_hr.ts \
    $$PWD/translations/qtermwidget_hu.ts \
    $$PWD/translations/qtermwidget_it.ts \
    $$PWD/translations/qtermwidget_ja.ts \
    $$PWD/translations/qtermwidget_ko.ts \
    $$PWD/translations/qtermwidget_lt.ts \
    $$PWD/translations/qtermwidget_nl.ts \
    $$PWD/translations/qtermwidget_nb_NO.ts \
    $$PWD/translations/qtermwidget_pl.ts \
    $$PWD/translations/qtermwidget_pt_BR.ts \
    $$PWD/translations/qtermwidget_pt.ts \
    $$PWD/translations/qtermwidget_ru.ts \
    $$PWD/translations/qtermwidget_si.ts \
    $$PWD/translations/qtermwidget_tr.ts \
    $$PWD/translations/qtermwidget.ts \
    $$PWD/translations/qtermwidget_sk.ts \
    $$PWD/translations/qtermwidget_uk.ts \
    $$PWD/translations/qtermwidget_zh_CN.ts \
    $$PWD/translations/qtermwidget_zh_TW.ts

RESOURCES += \
    $$PWD/res.qrc