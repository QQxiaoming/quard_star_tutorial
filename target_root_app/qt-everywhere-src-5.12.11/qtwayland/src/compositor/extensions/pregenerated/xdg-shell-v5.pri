# Putting pregenerated files in a 3rdparty directory to avoid
# qtqa's license checking failing.

HEADERS += \
    $$PWD/3rdparty/qwayland-server-xdg-shell-unstable-v5_p.h \
    $$PWD/3rdparty/wayland-xdg-shell-unstable-v5-server-protocol_p.h

SOURCES += \
    $$PWD/3rdparty/qwayland-server-xdg-shell-unstable-v5.cpp \
    $$PWD/3rdparty/wayland-xdg-shell-unstable-v5-protocol.c
