INCLUDEPATH += $$PWD

QMAKE_USE += wayland-client

SOURCES += \
        $$PWD/shmserverbufferintegration.cpp

HEADERS += \
        $$PWD/shmserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/shm-emulation-server-buffer.xml
