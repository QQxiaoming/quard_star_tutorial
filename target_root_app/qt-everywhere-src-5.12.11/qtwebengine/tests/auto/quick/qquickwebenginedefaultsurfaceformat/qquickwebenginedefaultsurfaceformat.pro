include(../tests.pri)

exists($${TARGET}.qrc):RESOURCES += $${TARGET}.qrc
QT_PRIVATE += core-private webengine-private webenginecore-private

HEADERS += ../shared/util.h
