mac {
    TEMPLATE = subdirs
    SUBDIRS += macextras
}
else {
    # fake project for creating the documentation
    TEMPLATE = aux
    CONFIG += force_qt
    QMAKE_DOCS = $$PWD/macextras/doc/qtmacextras.qdocconf
}
