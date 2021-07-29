TARGET    =  QtQmlDebug
QT        =  core-private network packetprotocol-private
CONFIG    += static internal_module

load(qt_module)

SOURCES += \
    qqmldebugclient.cpp \
    qqmldebugconnection.cpp \
    qqmldebugmessageclient.cpp \
    qqmlenginecontrolclient.cpp \
    qqmlpreviewclient.cpp \
    qqmlprofilerclient.cpp \
    qqmlprofilerevent.cpp \
    qqmlprofilereventlocation.cpp \
    qqmlprofilereventtype.cpp \
    qqmlprofilertypedevent.cpp

HEADERS += \
    qqmldebugclient_p.h \
    qqmldebugclient_p_p.h \
    qqmldebugconnection_p.h \
    qqmldebugmessageclient_p.h \
    qqmlenginecontrolclient_p.h \
    qqmlenginecontrolclient_p_p.h \
    qqmlpreviewclient_p.h \
    qqmlpreviewclient_p_p.h \
    qqmlprofilerclient_p.h \
    qqmlprofilerclient_p_p.h \
    qqmlprofilerevent_p.h \
    qqmlprofilereventlocation_p.h \
    qqmlprofilereventreceiver_p.h \
    qqmlprofilereventtype_p.h \
    qqmlprofilertypedevent_p.h \
    qqmlprofilerclientdefinitions_p.h
