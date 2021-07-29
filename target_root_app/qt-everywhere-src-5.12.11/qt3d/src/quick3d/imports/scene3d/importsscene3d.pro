CXX_MODULE = qml
TARGET = qtquickscene3dplugin
TARGETPATH = QtQuick/Scene3D
IMPORT_VERSION = 2.$$QT_MINOR_VERSION

QT += qml quick 3dcore 3drender 3drender-private

qtConfig(qt3d-input): QT += 3dinput
qtConfig(qt3d-logic): QT += 3dlogic
qtConfig(qt3d-animation): QT += 3danimation

# Qt3D is free of Q_FOREACH - make sure it stays that way:
DEFINES += QT_NO_FOREACH

HEADERS += \
    qtquickscene3dplugin.h \
    scene3dlogging_p.h \
    scene3ditem_p.h \
    scene3drenderer_p.h \
    scene3dsgnode_p.h \
    scene3dsgmaterialshader_p.h \
    scene3dsgmaterial_p.h

SOURCES += \
    qtquickscene3dplugin.cpp \
    scene3ditem.cpp \
    scene3dlogging.cpp \
    scene3drenderer.cpp \
    scene3dsgnode.cpp \
    scene3dsgmaterialshader.cpp \
    scene3dsgmaterial.cpp

OTHER_FILES += qmldir

load(qml_plugin)
