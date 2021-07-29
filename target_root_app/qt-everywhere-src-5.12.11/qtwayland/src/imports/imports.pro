TEMPLATE = subdirs

qtHaveModule(quick): {
    SUBDIRS += \
        compositor \
        texture-sharing \
        texture-sharing-extension
}
