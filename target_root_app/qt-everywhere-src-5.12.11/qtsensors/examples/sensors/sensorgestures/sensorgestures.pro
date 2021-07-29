TEMPLATE = app
TARGET = gesture
QT       += sensors widgets
requires(qtConfig(treewidget))

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

target.path = $$[QT_INSTALL_EXAMPLES]/sensors/sensorgestures
INSTALLS += target
