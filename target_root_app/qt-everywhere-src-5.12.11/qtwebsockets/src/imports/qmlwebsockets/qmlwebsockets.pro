QT = websockets qml-private core-private

TARGETPATH = QtWebSockets

HEADERS +=  qmlwebsockets_plugin.h \
            qqmlwebsocket.h \
            qqmlwebsocketserver.h

SOURCES +=  qmlwebsockets_plugin.cpp \
            qqmlwebsocket.cpp \
            qqmlwebsocketserver.cpp

OTHER_FILES += qmldir

IMPORT_VERSION = 1.1

load(qml_plugin)
