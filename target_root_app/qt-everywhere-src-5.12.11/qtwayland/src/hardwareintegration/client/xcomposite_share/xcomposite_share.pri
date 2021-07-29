INCLUDEPATH += $$PWD

QMAKE_USE += xcomposite x11
CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/xcomposite.xml

HEADERS += \
    $$PWD/qwaylandxcompositebuffer.h

SOURCES += \
    $$PWD/qwaylandxcompositebuffer.cpp
