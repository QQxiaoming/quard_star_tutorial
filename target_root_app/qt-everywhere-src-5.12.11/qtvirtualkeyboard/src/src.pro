TEMPLATE = subdirs

include(config.pri)

SUBDIRS += \
    virtualkeyboard \
    import \
    settings \
    styles \
    plugin \
    plugins

import.depends += virtualkeyboard
settings.depends += virtualkeyboard
styles.depends += virtualkeyboard
plugin.depends += virtualkeyboard
plugins.depends += virtualkeyboard
