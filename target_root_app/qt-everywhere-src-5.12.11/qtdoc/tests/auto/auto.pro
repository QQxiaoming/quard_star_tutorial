TEMPLATE=subdirs
qtHaveModule(quick) {
    SUBDIRS += quick
}
qtHaveModule(qml) {
    SUBDIRS += qml
}

