TEMPLATE = subdirs

!qtHaveModule(3dcore): \
    return()

QT_FOR_CONFIG += 3dcore

!package {
    SUBDIRS += auto
    qtConfig(qt3d-extras): SUBDIRS += manual
}

# Benchmarks make sense in release mode only.
# Disable them for code coverage.
!testcocoon: contains(QT_CONFIG,release): SUBDIRS += benchmarks
