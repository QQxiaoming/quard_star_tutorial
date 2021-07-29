TEMPLATE = app

QT += widgets axcontainer

HEADERS  = mediaaxwidget.h
SOURCES  = main.cpp
FORMS    = mainwindow.ui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/activeqt/mediaplayer
INSTALLS += target
