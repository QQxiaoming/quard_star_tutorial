include(../tests.pri)
QT += webengine
QT_PRIVATE += core-private webengine-private webenginecore-private
DEFINES += IMPORT_DIR=\"\\\"$${ROOT_BUILD_DIR}$${QMAKE_DIR_SEP}imports\\\"\"
