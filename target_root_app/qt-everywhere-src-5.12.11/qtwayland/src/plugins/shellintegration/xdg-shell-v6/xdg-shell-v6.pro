QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client
qtConfig(xkbcommon): \
    QMAKE_USE_PRIVATE += xkbcommon

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/xdg-shell-unstable-v6.xml

HEADERS += \
    qwaylandxdgshellv6_p.h \
    qwaylandxdgshellv6integration_p.h \

SOURCES += \
    main.cpp \
    qwaylandxdgshellv6.cpp \
    qwaylandxdgshellv6integration.cpp \

OTHER_FILES += \
    xdg-shell-v6.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandXdgShellV6IntegrationPlugin
load(qt_plugin)
