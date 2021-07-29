TARGET = QtQuickShapes

QT = core gui-private qml quick-private

CONFIG += simd optimize_full internal_module

HEADERS += \
    qquickshapesglobal.h \
    qquickshapesglobal_p.h \
    qquickshape_p.h \
    qquickshape_p_p.h \
    qquickshapegenericrenderer_p.h \
    qquickshapesoftwarerenderer_p.h

SOURCES += \
    qquickshape.cpp \
    qquickshapegenericrenderer.cpp \
    qquickshapesoftwarerenderer.cpp

qtConfig(opengl) {
    HEADERS += \
        qquicknvprfunctions_p.h \
        qquicknvprfunctions_p_p.h \
        qquickshapenvprrenderer_p.h

    SOURCES += \
        qquicknvprfunctions.cpp \
        qquickshapenvprrenderer.cpp
}

RESOURCES += qtquickshapes.qrc

load(qt_module)
