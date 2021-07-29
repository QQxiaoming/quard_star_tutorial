INCLUDEPATH += $$PWD

QMAKE_USE_PRIVATE += egl wayland-server

SOURCES += \
    $$PWD/dmabufserverbufferintegration.cpp


HEADERS += \
    $$PWD/dmabufserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../extensions/qt-dmabuf-server-buffer.xml
