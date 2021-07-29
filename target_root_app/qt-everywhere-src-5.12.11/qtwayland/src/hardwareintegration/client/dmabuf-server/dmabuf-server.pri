INCLUDEPATH += $$PWD

QMAKE_USE += egl wayland-client

SOURCES += \
        $$PWD/dmabufserverbufferintegration.cpp

HEADERS += \
        $$PWD/dmabufserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/qt-dmabuf-server-buffer.xml
