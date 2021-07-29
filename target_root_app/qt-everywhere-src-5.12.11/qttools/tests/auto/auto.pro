TEMPLATE=subdirs
SUBDIRS=\
    linguist \
    qtattributionsscanner \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \
    cmake \
    installed_cmake \
    qtdiag \
    windeployqt

installed_cmake.depends = cmake

# These tests don't make sense for cross-compiled builds
cross_compile:SUBDIRS -= linguist qtdiag

# These tests need the QtHelp module
!qtHaveModule(help): SUBDIRS -= \
    qhelpcontentmodel \
    qhelpenginecore \
    qhelpgenerator \
    qhelpindexmodel \
    qhelpprojectdata \

!qtConfig(process): SUBDIRS -= qtattributionsscanner linguist qtdiag windeployqt
!win32|winrt: SUBDIRS -= windeployqt
