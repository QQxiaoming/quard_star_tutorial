QT += core-private multimedia-private multimediagsttools-private network

qtHaveModule(widgets) {
    QT += widgets multimediawidgets-private
    DEFINES += HAVE_WIDGETS
}

QMAKE_USE += gstreamer

qtConfig(resourcepolicy): \
    QMAKE_USE += libresourceqt5

qtConfig(gstreamer_app): \
    QMAKE_USE += gstreamer_app

