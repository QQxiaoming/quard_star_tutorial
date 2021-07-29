TEMPLATE = subdirs

SUBDIRS += \
    3rdparty/lipi-toolkit \
    plugin

plugin.depends += 3rdparty/lipi-toolkit
