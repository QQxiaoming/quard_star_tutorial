TEMPLATE = lib
TARGET = Imports
QT += qml
CONFIG += qt plugin

CONFIG -= debug_and_release_target
!build_pass:qtConfig(debug_and_release): CONFIG += release

TARGET = $$qtLibraryTarget($$TARGET)

SOURCES += \
    imports_plugin.cpp \
    imports.cpp

HEADERS += \
    imports_plugin.h \
    imports.h

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    cp.files = qmldir plugins.qmltypes CompositeImports.qml
    cp.path = $$OUT_PWD
    COPIES += cp
}

