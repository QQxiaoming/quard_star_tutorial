QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client
qtConfig(xkbcommon): \
    QMAKE_USE_PRIVATE += xkbcommon

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/xdg-decoration-unstable-v1.xml \
    ../../../3rdparty/protocol/xdg-shell.xml

HEADERS += \
    qwaylandxdgdecorationv1_p.h \
    qwaylandxdgshell_p.h \
    qwaylandxdgshellintegration_p.h \

SOURCES += \
    main.cpp \
    qwaylandxdgdecorationv1.cpp \
    qwaylandxdgshell.cpp \
    qwaylandxdgshellintegration.cpp \

OTHER_FILES += \
    xdg-shell.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandXdgShellIntegrationPlugin
load(qt_plugin)
