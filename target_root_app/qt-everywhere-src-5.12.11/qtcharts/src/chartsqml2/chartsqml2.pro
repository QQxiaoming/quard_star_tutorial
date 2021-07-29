
TARGET = qtchartsqml2
CXX_MODULE = charts
DEFINES += QT_BUILD_QMLCHARTS_LIB

QT += qml quick
QT += charts charts-private
contains(QT_COORD_TYPE, float): DEFINES += QT_QREAL_IS_FLOAT

include($$PWD/designer/designer.pri)

SOURCES += \
    chartsqml2_plugin.cpp \
    declarativechart.cpp \
    declarativexypoint.cpp \
    declarativexyseries.cpp \
    declarativelineseries.cpp \
    declarativesplineseries.cpp \
    declarativeareaseries.cpp \
    declarativescatterseries.cpp \
    declarativepieseries.cpp \
    declarativebarseries.cpp \
    declarativecategoryaxis.cpp \
    declarativemargins.cpp \
    declarativeaxes.cpp \
    declarativepolarchart.cpp \
    declarativeboxplotseries.cpp \
    declarativechartnode.cpp \
    declarativecandlestickseries.cpp

PRIVATE_HEADERS += \
    declarativechart_p.h \
    declarativexypoint_p.h \
    declarativexyseries_p.h \
    declarativelineseries_p.h \
    declarativesplineseries_p.h \
    declarativeareaseries_p.h \
    declarativescatterseries_p.h \
    declarativepieseries_p.h \
    declarativebarseries_p.h \
    declarativecategoryaxis_p.h \
    declarativemargins_p.h \
    declarativeaxes_p.h \
    declarativepolarchart_p.h \
    declarativeboxplotseries_p.h \
    declarativecandlestickseries_p.h \
    declarativeabstractrendernode_p.h \
    declarativechartnode_p.h \
    declarativechartglobal_p.h

contains(QT_CONFIG, opengl) {
    SOURCES += declarativeopenglrendernode.cpp
    PRIVATE_HEADERS += declarativeopenglrendernode_p.h
}

HEADERS += $$PRIVATE_HEADERS
INCLUDEPATH += $$PWD

IMPORT_VERSION = 2.3
QMAKE_QMLPLUGINDUMP_FLAGS = -qapp

load(qml_plugin)

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Charts (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Charts QML plugin for Qt."
}
