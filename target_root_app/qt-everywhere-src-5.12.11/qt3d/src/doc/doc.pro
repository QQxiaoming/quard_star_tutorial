TEMPLATE = aux

CONFIG += force_qt

# Add Qt 3D modules for which QDoc needs include paths passed
QT += \
    core-private \
    3dcore-private \
    3drender-private \
    3dinput-private \
    3danimation-private

QMAKE_DOCS = $$PWD/qt3d.qdocconf

OTHER_FILES += $$PWD/src/*.qdoc
