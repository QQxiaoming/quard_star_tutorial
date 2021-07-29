TEMPLATE = lib
TARGET = Versions
QT += qml
CONFIG += qt plugin

CONFIG -= debug_and_release_target
!build_pass:qtConfig(debug_and_release): CONFIG += release

TARGET = $$qtLibraryTarget($$TARGET)

SOURCES += \
    versions_plugin.cpp \
    versions.cpp

HEADERS += \
    versions_plugin.h \
    versions.h

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    cpqmldir.files = qmldir plugins.qmltypes
    cpqmldir.path = $$OUT_PWD
    COPIES += cpqmldir
}
