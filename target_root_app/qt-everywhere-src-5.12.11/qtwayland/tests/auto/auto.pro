TEMPLATE=subdirs
QT_FOR_CONFIG += waylandclient-private

!qtHaveModule(waylandcompositor): \
    return()

SUBDIRS += compositor

qtConfig(wayland-client): \
    SUBDIRS += client cmake
