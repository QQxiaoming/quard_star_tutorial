TARGET   = Qt3DQuickExtras
MODULE   = 3dquickextras

QT      += core core-private qml qml-private 3dcore 3dinput 3dquick 3dquick-private 3drender 3drender-private 3dlogic 3dextras

CONFIG -= precompile_header

# Qt3D is free of Q_FOREACH - make sure it stays that way:
DEFINES += QT_NO_FOREACH

gcov {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
}

SOURCES += \
    qt3dquickextras_global.cpp \
    qt3dquickextrasnodefactory.cpp \
    qt3dquickwindow.cpp \
    qt3dquickwindowlogging.cpp

HEADERS += \
    qt3dquickextras_global.h \
    qt3dquickextras_global_p.h \
    qt3dquickextrasnodefactory_p.h \
    qt3dquickwindow.h \
    qt3dquickwindow_p.h \
    qt3dquickwindowlogging_p.h

include(./items/items.pri)

load(qt_module)
