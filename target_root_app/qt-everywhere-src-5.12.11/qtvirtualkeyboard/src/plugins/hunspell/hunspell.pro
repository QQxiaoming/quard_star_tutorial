TEMPLATE = subdirs

include(../../config.pri)

SUBDIRS += \
    hunspellinputmethod \
    plugin
hunspell-library {
    SUBDIRS += 3rdparty/hunspell
    hunspellinputmethod.depends += 3rdparty/hunspell
}

plugin.depends += hunspellinputmethod
