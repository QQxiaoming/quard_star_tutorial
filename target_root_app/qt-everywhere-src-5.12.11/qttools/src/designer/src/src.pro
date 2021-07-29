TEMPLATE = subdirs

SUBDIRS = \
    uiplugin \
    uitools

qtConfig(process) {
    SUBDIRS += \
        lib \
        components \
        designer

    lib.depends = uiplugin
    components.depends = lib
    designer.depends = components
    plugins.depends = lib

    contains(QT_CONFIG, shared): SUBDIRS += plugins
}

uitools.depends = uiplugin

qtNomakeTools( \
    lib \
    components \
    designer \
    plugins \
)
