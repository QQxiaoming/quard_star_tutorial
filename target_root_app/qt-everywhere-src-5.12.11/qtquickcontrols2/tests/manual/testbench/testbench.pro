TEMPLATE = app

QT += qml quick quickcontrols2
CONFIG += c++11

HEADERS += \
    assetfixer.h \
    directoryvalidator.h \
    clipboard.h

SOURCES += main.cpp \
    assetfixer.cpp \
    directoryvalidator.cpp \
    clipboard.cpp

RESOURCES += qml.qrc \
    fonts.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

