INCLUDEPATH += \
        -I $$PWD/utf8proc \
        -I $$PWD/util/ \
        -I $$PWD 

SOURCES += \
    $$PWD/utf8proc/utf8proc.c \
    $$PWD/utf8proc/utf8proc_data.c \
    $$PWD/util/CharWidth.cpp \
    $$PWD/util/ColorScheme.cpp \
    $$PWD/util/Filter.cpp \
    $$PWD/util/History.cpp \
    $$PWD/util/HistorySearch.cpp \
    $$PWD/util/KeyboardTranslator.cpp \
    $$PWD/util/SearchBar.cpp \
    $$PWD/util/TerminalCharacterDecoder.cpp \
    $$PWD/Emulation.cpp \
    $$PWD/Vt102Emulation.cpp \
    $$PWD/Screen.cpp \
    $$PWD/ScreenWindow.cpp \
    $$PWD/TerminalDisplay.cpp \
    $$PWD/qtermwidget.cpp

HEADERS += \
    $$PWD/utf8proc/utf8proc.h \
    $$PWD/util/CharWidth.h \
    $$PWD/util/CharacterColor.h \
    $$PWD/util/Character.h \
    $$PWD/util/ColorScheme.h \
    $$PWD/util/Filter.h \
    $$PWD/util/History.h \
    $$PWD/util/HistorySearch.h \
    $$PWD/util/KeyboardTranslator.h \
    $$PWD/util/SearchBar.h \
    $$PWD/util/TerminalCharacterDecoder.h \
    $$PWD/Emulation.h \
    $$PWD/Vt102Emulation.h \
    $$PWD/Screen.h \
    $$PWD/ScreenWindow.h \
    $$PWD/TerminalDisplay.h \
    $$PWD/qtermwidget.h \
    $$PWD/qtermwidget_version.h

FORMS += \
    $$PWD/util/SearchBar.ui
    
RESOURCES += \
    $$PWD/res.qrc
