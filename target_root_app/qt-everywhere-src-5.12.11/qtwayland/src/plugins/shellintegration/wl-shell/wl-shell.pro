QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client
qtConfig(xkbcommon): \
    QMAKE_USE_PRIVATE += xkbcommon

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/wayland.xml

HEADERS += \
    qwaylandwlshellintegration_p.h \
    qwaylandwlshellsurface_p.h \

SOURCES += \
    main.cpp \
    qwaylandwlshellintegration.cpp \
    qwaylandwlshellsurface.cpp \

OTHER_FILES += \
    wl-shell.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandWlShellIntegrationPlugin
load(qt_plugin)
