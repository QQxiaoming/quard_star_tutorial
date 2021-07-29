TARGET   = Qt3DQuickAnimation
MODULE   = 3dquickanimation

QT      += core core-private qml qml-private 3dcore 3drender 3danimation 3dquick 3dquick-private 3dcore-private 3drender-private
CONFIG -= precompile_header

# Qt3D is free of Q_FOREACH - make sure it stays that way:
DEFINES += QT_NO_FOREACH

gcov {
    CONFIG += static
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
}

SOURCES += \
    qt3dquickanimation_global.cpp \
    qt3dquickanimationnodefactory.cpp

HEADERS += \
    qt3dquickanimationnodefactory_p.h \
    qt3dquickanimation_global_p.h \
    qt3dquickanimation_global.h

include(./items/items.pri)

load(qt_module)
