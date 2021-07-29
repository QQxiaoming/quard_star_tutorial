TEMPLATE = subdirs

qtHaveModule(quick) {
    SUBDIRS += \
        samegame \
        calqlatr \
        clocks \
        tweetsearch \
        maroon \
        photosurface \
        stocqt

    qtHaveModule(quickcontrols2) {
        SUBDIRS += coffee
    }

    qtHaveModule(xmlpatterns) {
        SUBDIRS += rssnews
        qtHaveModule(quickcontrols) {
            SUBDIRS += photoviewer
        }
    }
}
