TEMPLATE = subdirs

!android: SUBDIRS += cpptest

qtHaveModule(quick): SUBDIRS += qmltest

installed_cmake.depends = cmake

# OpenGL support is needed for all qmltests but it's not available on boot2qt
boot2qt: SUBDIRS -= qmltest
