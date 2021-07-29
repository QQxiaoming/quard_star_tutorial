qtConfig(qml-debug) {
    HEADERS += \
        $$PWD/qqmlabstractprofileradapter_p.h \
        $$PWD/qqmlconfigurabledebugservice_p.h \
        $$PWD/qqmldebugpluginmanager_p.h \
        $$PWD/qqmldebugservice_p.h \
        $$PWD/qqmldebugservicefactory_p.h \
        $$PWD/qqmldebugserver_p.h \
        $$PWD/qqmldebugserverconnection_p.h \
        $$PWD/qqmlprofilerdefinitions_p.h

    SOURCES += \
        $$PWD/qqmldebug.cpp \
        $$PWD/qqmldebugconnector.cpp \
        $$PWD/qqmldebugservice.cpp \
        $$PWD/qqmlabstractprofileradapter.cpp \
        $$PWD/qqmlmemoryprofiler.cpp \
        $$PWD/qqmlprofiler.cpp \
        $$PWD/qqmldebugserviceinterfaces.cpp
}

HEADERS += \
    $$PWD/qqmldebugconnector_p.h \
    $$PWD/qqmldebugserviceinterfaces_p.h \
    $$PWD/qqmldebugstatesdelegate_p.h \
    $$PWD/qqmldebug.h \
    $$PWD/qqmlmemoryprofiler_p.h \
    $$PWD/qqmlprofiler_p.h

INCLUDEPATH += $$PWD
