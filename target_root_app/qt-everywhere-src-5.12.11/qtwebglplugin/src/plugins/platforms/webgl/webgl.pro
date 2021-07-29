TARGET = qwebgl
QT += \
    websockets \
    gui-private \
    eventdispatcher_support-private \
    fontdatabase_support-private \
    theme_support-private

qtHaveModule(quick) {
    QT += quick
}

HEADERS += \
    qwebglcontext.h \
    qwebglfunctioncall.h \
    qwebglhttpserver.h \
    qwebglintegration.h \
    qwebglintegration_p.h \
    qwebglplatformservices.h \
    qwebglscreen.h \
    qwebglwebsocketserver.h \
    qwebglwindow.h \
    qwebglwindow_p.h

SOURCES += \
    qwebglcontext.cpp \
    qwebglfunctioncall.cpp \
    qwebglhttpserver.cpp \
    qwebglintegration.cpp \
    qwebglmain.cpp \
    qwebglplatformservices.cpp \
    qwebglscreen.cpp \
    qwebglwebsocketserver.cpp \
    qwebglwindow.cpp

RESOURCES += \
    webgl.qrc

DISTFILES += webgl.json

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = QWebGLIntegrationPlugin
!equals(TARGET, $$QT_DEFAULT_QPA_PLUGIN): PLUGIN_EXTENDS = -
load(qt_plugin)
