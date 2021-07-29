TEMPLATE = app

QT += widgets texttospeech
requires(qtHaveModule(gui))
qtHaveModule(gui): requires(qtConfig(combobox))

SOURCES = main.cpp \
          mainwindow.cpp
HEADERS = mainwindow.h

FORMS = mainwindow.ui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/speech/hello_speak
INSTALLS += target
