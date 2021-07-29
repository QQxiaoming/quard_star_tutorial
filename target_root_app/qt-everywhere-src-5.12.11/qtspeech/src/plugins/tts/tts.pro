TEMPLATE = subdirs

unix {
    CONFIG += link_pkgconfig
    config_speechd | packagesExist(speech-dispatcher) {
        SUBDIRS += speechdispatcher
    }
}

windows:!winrt: SUBDIRS += sapi
winrt: SUBDIRS += winrt

osx: SUBDIRS += osx
uikit: SUBDIRS += ios

android: SUBDIRS += android

config_flite | config_flite_alsa {
    qtHaveModule(multimedia): SUBDIRS += flite
}
