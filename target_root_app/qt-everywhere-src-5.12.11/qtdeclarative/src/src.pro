TEMPLATE = subdirs
CONFIG += ordered
include($$OUT_PWD/qml/qtqml-config.pri)
include($$OUT_PWD/quick/qtquick-config.pri)
QT_FOR_CONFIG += qml qml-private quick-private
SUBDIRS += \
    qml

qtHaveModule(gui):qtConfig(qml-animation) {
    SUBDIRS += \
        quick \
        quickshapes

    qtConfig(testlib): \
        SUBDIRS += qmltest

    qtConfig(quick-particles): \
        SUBDIRS += particles
    qtHaveModule(widgets): SUBDIRS += quickwidgets
}

SUBDIRS += \
    plugins \
    imports

qtConfig(qml-devtools): SUBDIRS += qmldevtools

qmldevtools.depends = qml

qtConfig(qml-network) {
    QT_FOR_CONFIG += network
    qtConfig(thread):qtConfig(localserver):qtConfig(qml-debug): SUBDIRS += qmldebug
}

DISTFILES += sync.profile configure.json
