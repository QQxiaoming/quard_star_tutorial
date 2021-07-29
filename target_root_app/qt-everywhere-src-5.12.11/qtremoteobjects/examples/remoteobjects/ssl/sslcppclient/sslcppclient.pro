QT_FOR_CONFIG += network
requires(qtConfig(ssl))

REPC_REPLICA += timemodel.rep
QT = remoteobjects remoteobjects-private core

QT       -= gui

TARGET = SslCppClient
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

OTHER_FILES += \
    timemodel.rep

RESOURCES += \
    ../sslserver/cert/cert.qrc


target.path = $$[QT_INSTALL_EXAMPLES]/remoteobjects/ssl/sslcppclient
INSTALLS += target
