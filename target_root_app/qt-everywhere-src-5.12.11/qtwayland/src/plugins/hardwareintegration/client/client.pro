TEMPLATE=subdirs
QT_FOR_CONFIG += waylandclient-private

qtConfig(wayland-egl): \
    SUBDIRS += wayland-egl
qtConfig(wayland-brcm): \
    SUBDIRS += brcm-egl
qtConfig(xcomposite-egl): \
    SUBDIRS += xcomposite-egl
qtConfig(xcomposite-glx): \
    SUBDIRS += xcomposite-glx

qtConfig(wayland-drm-egl-server-buffer): \
    SUBDIRS += drm-egl-server
qtConfig(wayland-libhybris-egl-server-buffer): \
    SUBDIRS += libhybris-egl-server
qtConfig(wayland-shm-emulation-server-buffer): \
    SUBDIRS += shm-emulation-server
qtConfig(wayland-dmabuf-server-buffer): \
    SUBDIRS += dmabuf-server
qtConfig(wayland-vulkan-server-buffer): \
    SUBDIRS += vulkan-server
