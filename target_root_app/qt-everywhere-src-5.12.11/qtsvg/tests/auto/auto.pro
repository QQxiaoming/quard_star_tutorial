TEMPLATE=subdirs
qtHaveModule(widgets) {
    SUBDIRS = \
        qsvgdevice \
        qsvggenerator \
        qsvgplugin \
        qicon_svg \
        cmake \
        installed_cmake

    !android: SUBDIRS += qsvgrenderer

    installed_cmake.depends = cmake
}
!cross_compile: SUBDIRS += host.pro
