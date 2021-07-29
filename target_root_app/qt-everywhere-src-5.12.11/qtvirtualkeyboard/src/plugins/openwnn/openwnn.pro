TEMPLATE = subdirs

SUBDIRS += \
    3rdparty/openwnn \
    plugin

plugin.depends += 3rdparty/openwnn
