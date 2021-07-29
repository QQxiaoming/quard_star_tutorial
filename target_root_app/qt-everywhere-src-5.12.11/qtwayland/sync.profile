%modules = ( # path to module name map
    "QtWaylandCompositor" => "$basedir/src/compositor",
    "QtWaylandClient" => "$basedir/src/client",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%deprecatedheaders = (
    "QtWaylandClient" =>  {
        "qwaylandclientexport.h" => "QtWaylandClient/qtwaylandclientglobal.h"
    },
    "QtWaylandCompositor" =>  {
        "qwaylandexport.h" => "QtWaylandCompositor/qtwaylandcompositorglobal.h"
    }
);
%classnames = (
    "qwaylandquickextension.h" => "QWaylandQuickExtension",
);
%inject_headers = (
    "$basedir/src/client" => [
        "^qwayland-hardware-integration.h",
        "^qwayland-qt-windowmanager.h",
        "^qwayland-qt-key-unstable-v1.h" ,
        "^qwayland-server-buffer-extension.h",
        "^qwayland-surface-extension.h",
        "^qwayland-text-input-unstable-v2.h",
        "^qwayland-touch-extension.h",
        "^qwayland-wayland.h",
        "^qwayland-xdg-output-unstable-v1.h",
        "^wayland-hardware-integration-client-protocol.h",
        "^wayland-qt-windowmanager-client-protocol.h",
        "^wayland-qt-key-unstable-v1-client-protocol.h",
        "^wayland-server-buffer-extension-client-protocol.h",
        "^wayland-surface-extension-client-protocol.h",
        "^wayland-text-input-unstable-v2-client-protocol.h",
        "^wayland-touch-extension-client-protocol.h",
        "^wayland-wayland-client-protocol.h",
        "^wayland-xdg-output-unstable-v1-client-protocol.h",
    ],
    "$basedir/src/plugins/shellintegration/xdg-shell" => [
        "^qwayland-xdg-shell.h",
        "^qwayland-xdg-decoration-unstable-v1.h",
        "^wayland-xdg-shell-client-protocol.h",
        "^wayland-xdg-decoration-unstable-v1-client-protocol.h",
    ],
    "$basedir/src/plugins/shellintegration/xdg-shell-v6" => [
        "^qwayland-xdg-shell-unstable-v6.h",
        "^wayland-xdg-shell-unstable-v6-client-protocol.h",
    ],
    "$basedir/src/compositor" => [
        "^qwayland-server-wayland.h",
        "^qwayland-server-hardware-integration.h",
        "^qwayland-server-ivi-application.h",
        "^qwayland-server-qt-windowmanager.h",
        "^qwayland-server-qt-key-unstable-v1.h",
        "^qwayland-server-qt-texture-sharing-unstable-v1.h",
        "^qwayland-server-server-buffer-extension.h",
        "^qwayland-server-text-input-unstable-v2.h",
        "^qwayland-server-touch-extension.h",
        "^qwayland-server-xdg-decoration-unstable-v1.h",
        "^qwayland-server-xdg-shell-unstable-v6.h",
        "^qwayland-server-xdg-shell.h",
        "^wayland-hardware-integration-server-protocol.h",
        "^wayland-ivi-application-server-protocol.h",
        "^wayland-qt-windowmanager-server-protocol.h",
        "^wayland-qt-key-unstable-v1-server-protocol.h",
        "^wayland-qt-texture-sharing-unstable-v1-server-protocol.h",
        "^wayland-server-buffer-extension-server-protocol.h",
        "^wayland-text-input-unstable-v2-server-protocol.h",
        "^wayland-touch-extension-server-protocol.h",
        "^wayland-wayland-server-protocol.h",
        "^wayland-xdg-decoration-unstable-v1-server-protocol.h",
        "^wayland-xdg-shell-server-protocol.h",
        "^wayland-xdg-shell-unstable-v6-server-protocol.h",
    ],
);
@private_headers = ( qr/^qwayland-.*\.h/, qr/^wayland-.*-protocol\.h/ );
