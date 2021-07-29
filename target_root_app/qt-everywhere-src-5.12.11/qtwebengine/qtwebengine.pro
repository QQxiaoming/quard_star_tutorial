load(qt_parts)
load(configure)

runConfigure()

!isEmpty(skipBuildReason) {
    SUBDIRS =
    log($${skipBuildReason}$${EOL})
    log(QtWebEngine will not be built.$${EOL})
}

QMAKE_DISTCLEAN += .qmake.cache

OTHER_FILES = \
    tools/buildscripts/* \
    tools/scripts/* \
    config.tests/khr/* \
    config.tests/libcap/* \
    config.tests/libvpx/* \
    config.tests/snappy/* \
    config.tests/re2/* \
    mkspecs/features/*
