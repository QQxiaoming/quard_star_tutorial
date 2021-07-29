include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore webenginecore-private

TEMPLATE = subdirs

SUBDIRS += \
    defaultsurfaceformat \
    devtools \
    faviconmanager \
    loadsignals \
    origins \
    schemes \
    shutdown \
    qwebenginedownloaditem \
    qwebenginepage \
    qwebenginehistory \
    qwebengineprofile \
    qwebenginescript \
    qwebenginesettings \
    qwebengineview

qtConfig(accessibility) {
    SUBDIRS += accessibility
}

qtConfig(webengine-printing-and-pdf) {
    SUBDIRS += printing
}

# QTBUG-71229
linux:!boot2qt: SUBDIRS += proxypac

qtConfig(webengine-spellchecker):!cross_compile {
    !qtConfig(webengine-native-spellchecker) {
        SUBDIRS += spellchecking
    } else {
        message("Spellcheck test will not be built because it depends on usage of Hunspell dictionaries.")
    }
}

# QTBUG-60268
boot2qt: SUBDIRS -= accessibility defaultsurfaceformat devtools \
                    faviconmanager qwebenginepage qwebenginehistory \
                    qwebengineprofile qwebenginescript \
                    qwebengineview qwebenginedownloaditem qwebenginesettings \
                    schemes origins loadsignals
