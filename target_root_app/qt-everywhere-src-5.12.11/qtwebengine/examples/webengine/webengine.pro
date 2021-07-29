TEMPLATE=subdirs

SUBDIRS += \
    customdialogs \
    minimal \
    quicknanobrowser

qtHaveModule(quickcontrols2) {
    SUBDIRS += \
        recipebrowser
}
