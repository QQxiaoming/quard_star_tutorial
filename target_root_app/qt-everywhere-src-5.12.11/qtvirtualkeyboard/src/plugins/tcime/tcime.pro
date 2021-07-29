TEMPLATE = subdirs

SUBDIRS += \
    3rdparty/tcime \
    plugin

plugin.depends += 3rdparty/tcime
