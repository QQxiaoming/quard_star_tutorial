TEMPLATE = subdirs

android {
    SUBDIRS += android
} else:if(ios|macos) {
    SUBDIRS += darwin
} else:winrt {
    SUBDIRS += winrt
}

qtHaveModule(webengine) {
    SUBDIRS += webengine
}
