TARGET = QtPacketProtocol
QT     = core-private
CONFIG += static internal_module

HEADERS = \
    qpacketprotocol_p.h \
    qpacket_p.h \
    qversionedpacket_p.h

SOURCES = \
    qpacketprotocol.cpp \
    qpacket.cpp

load(qt_module)
