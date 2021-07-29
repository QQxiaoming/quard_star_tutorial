INCLUDEPATH += $$PWD

QMAKE_USE_PRIVATE += wayland-server

SOURCES += \
    $$PWD/shmserverbufferintegration.cpp


HEADERS += \
    $$PWD/shmserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDSERVERSOURCES += $$PWD/../../../extensions/shm-emulation-server-buffer.xml
