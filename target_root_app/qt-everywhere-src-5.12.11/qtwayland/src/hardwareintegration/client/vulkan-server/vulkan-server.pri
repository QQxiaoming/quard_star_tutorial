INCLUDEPATH += $$PWD

QMAKE_USE += wayland-client

SOURCES += \
        $$PWD/vulkanserverbufferintegration.cpp

HEADERS += \
        $$PWD/vulkanserverbufferintegration.h

CONFIG += wayland-scanner
WAYLANDCLIENTSOURCES += $$PWD/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
