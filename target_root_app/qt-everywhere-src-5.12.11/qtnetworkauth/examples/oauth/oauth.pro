TEMPLATE = subdirs

qtHaveModule(widgets) {
    SUBDIRS += \
        twittertimeline \
        redditclient
}
