TEMPLATE = app
TARGET = stylescoverflow
QT += quick quickcontrols2

SOURCES += \
    styles-cover-flow.cpp

RESOURCES += \
    $$files(*.qml, true)

DEFINES += DOC_IMAGES_DIR=\\\"$$PWD/../../../src/imports/controls/doc/images/\\\"
