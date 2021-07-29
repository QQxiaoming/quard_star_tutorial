INCLUDEPATH += $$PWD

QMAKE_USE_PRIVATE += egl wayland-server wayland-egl

QT += egl_support-private

SOURCES += \
    $$PWD/waylandeglclientbufferintegration.cpp

HEADERS += \
    $$PWD/waylandeglclientbufferintegration.h
