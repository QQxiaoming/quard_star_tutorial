TEMPLATE = lib
TARGET = ExtendedType
QT += qml
CONFIG += qt plugin

CONFIG -= debug_and_release_target
!build_pass:qtConfig(debug_and_release): CONFIG += release

TARGET = $$qtLibraryTarget($$TARGET)

SOURCES += \
    plugin.cpp

HEADERS += \
    plugin.h \
    types.h

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    cp.files = qmldir plugins.qmltypes
    cp.path = $$OUT_PWD
    COPIES += cp
}
