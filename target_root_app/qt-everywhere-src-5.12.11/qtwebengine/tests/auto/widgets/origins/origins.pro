include(../tests.pri)
CONFIG += c++14
qtConfig(webengine-webchannel):qtHaveModule(websockets) {
    QT += websockets
    DEFINES += WEBSOCKETS
}

