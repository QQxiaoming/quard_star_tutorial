TEMPLATE = subdirs

SUBDIRS += \
    3rdparty/pinyin \
    plugin

plugin.depends += 3rdparty/pinyin
