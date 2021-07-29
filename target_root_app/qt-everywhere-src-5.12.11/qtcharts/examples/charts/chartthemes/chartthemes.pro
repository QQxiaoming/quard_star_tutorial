QT += charts
requires(qtConfig(combobox))

HEADERS += \
    themewidget.h

SOURCES += \
    main.cpp \
    themewidget.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/charts/chartthemes
INSTALLS += target

FORMS += \
    themewidget.ui
