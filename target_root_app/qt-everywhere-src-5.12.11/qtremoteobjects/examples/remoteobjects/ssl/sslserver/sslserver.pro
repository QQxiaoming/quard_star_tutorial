QT_FOR_CONFIG += network
requires(qtConfig(ssl))

CONFIG   += console


REPC_SOURCE += ../../timemodel.rep
QT = remoteobjects remoteobjects-private core

SOURCES += timemodel.cpp main.cpp \
    sslserver.cpp
HEADERS += timemodel.h \
    sslserver.h

contains(QT_CONFIG, c++11): CONFIG += c++11

target.path = $$[QT_INSTALL_EXAMPLES]/remoteobjects/ssl/sslserver
INSTALLS += target

RESOURCES += \
    cert/cert.qrc
