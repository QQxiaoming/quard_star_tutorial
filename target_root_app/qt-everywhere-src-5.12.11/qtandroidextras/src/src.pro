android {
    TEMPLATE = subdirs
    SUBDIRS += androidextras jar
} else {
    TEMPLATE = aux
    CONFIG += force_qt
    QMAKE_DOCS = $$PWD/androidextras/doc/qtandroidextras.qdocconf
}
