TEMPLATE = subdirs

SUBDIRS += \
    3rdparty/t9write \
    plugin

plugin.depends += 3rdparty/t9write
