TEMPLATE = subdirs

SUBDIRS = \
    coretest \
    core \
    cmake \
    global

installed_cmake.depends = cmake

QT_FOR_CONFIG += 3dcore
qtConfig(qt3d-render): SUBDIRS += render
qtConfig(qt3d-input): SUBDIRS += input
qtConfig(qt3d-animation): SUBDIRS += animation
qtConfig(qt3d-extras): SUBDIRS += extras
qtConfig(qt3d-render) {
    SUBDIRS += geometryloaders
    qtConfig(qt3d-input): SUBDIRS += quick3d
}

for(subdir, SUBDIRS) {
    !equals(subdir, coretest) {
        $${subdir}.depends += coretest
    }
}
