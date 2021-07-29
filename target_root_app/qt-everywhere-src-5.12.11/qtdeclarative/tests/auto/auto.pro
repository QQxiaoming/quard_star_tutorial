TEMPLATE=subdirs
SUBDIRS=\
    qml \
    quick \
    quicktest \
    qmltest \
    qmldevtools \
    cmake \
    installed_cmake \
    toolsupport

qtHaveModule(gui):qtConfig(opengl(es1|es2)?) {
    SUBDIRS += particles
    qtHaveModule(widgets): SUBDIRS += quickwidgets

}

# console applications not supported
uikit: SUBDIRS -= qmltest

installed_cmake.depends = cmake
