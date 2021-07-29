CONFIG += testcase link_pkgconfig
QT += testlib
QT += core-private gui-private waylandclient-private

QMAKE_USE += wayland-client wayland-server

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += \
    ../../../../src/3rdparty/protocol/ivi-application.xml \
    ../../../../src/3rdparty/protocol/wayland.xml \
    ../../../../src/3rdparty/protocol/xdg-shell-unstable-v6.xml

INCLUDEPATH += ../shared

SOURCES += \
    ../shared/mockcompositor.cpp \
    ../shared/mockinput.cpp \
    ../shared/mockiviapplication.cpp \
    ../shared/mockwlshell.cpp \
    ../shared/mockxdgshellv6.cpp \
    ../shared/mocksurface.cpp \
    ../shared/mockoutput.cpp

HEADERS += \
    ../shared/mockcompositor.h \
    ../shared/mockinput.h \
    ../shared/mockiviapplication.h \
    ../shared/mockwlshell.h \
    ../shared/mockxdgshellv6.h \
    ../shared/mocksurface.h \
    ../shared/mockoutput.h
