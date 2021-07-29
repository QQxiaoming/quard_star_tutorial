TEMPLATE = lib
TARGET = Dummy
QT += qml
CONFIG += qt plugin

CONFIG -= debug_and_release_target
!build_pass:qtConfig(debug_and_release): CONFIG += release

TARGET = $$qtLibraryTarget($$TARGET)

SOURCES += \
    dummy_plugin.cpp \
    dummy.cpp

HEADERS += \
    dummy_plugin.h \
    dummy.h

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    cp.files = qmldir plugins.qmltypes
    cp.path = $$OUT_PWD
    COPIES += cp
}
