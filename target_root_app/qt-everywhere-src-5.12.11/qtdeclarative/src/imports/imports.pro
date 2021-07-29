TEMPLATE = subdirs

SUBDIRS += \
    builtins \
    qtqml \
    models \
    labsmodels

qtConfig(thread): SUBDIRS += folderlistmodel
qtHaveModule(sql): SUBDIRS += localstorage
qtConfig(settings): SUBDIRS += settings
qtConfig(statemachine): SUBDIRS += statemachine

qtHaveModule(quick) {
    QT_FOR_CONFIG += quick-private

    SUBDIRS += \
        layouts \
        qtquick2 \
        window

    qtConfig(quick-shadereffect): SUBDIRS += wavefrontmesh

    qtHaveModule(testlib): SUBDIRS += testlib
    qtConfig(systemsemaphore): SUBDIRS += sharedimage
    qtConfig(quick-particles): \
        SUBDIRS += particles

    qtConfig(quick-path):qtConfig(thread): SUBDIRS += shapes
}

