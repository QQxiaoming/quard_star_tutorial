option(host_build)
QT = core-private
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_FOREACH

include(../shared/winutils/winutils.pri)
SOURCES += main.cpp

CONFIG += force_bootstrap

win32: LIBS += -lshlwapi

QMAKE_TARGET_DESCRIPTION = "Qt Windows Deployment Tool"
load(qt_tool)
