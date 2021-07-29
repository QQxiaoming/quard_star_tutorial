TEMPLATE = subdirs

include($$OUT_PWD/../../serialbus/qtserialbus-config.pri)
QT_FOR_CONFIG += serialbus-private

SUBDIRS += virtualcan

qtConfig(socketcan) {
    SUBDIRS += socketcan
}

qtConfig(library) {
    SUBDIRS += passthrucan peakcan tinycan
    win32:SUBDIRS += systeccan vectorcan
}
