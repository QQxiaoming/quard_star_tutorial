TEMPLATE = subdirs

# OpenGL renderer
include($$OUT_PWD/qt3drender-config.pri)
QT_FOR_CONFIG += 3drender-private

qtConfig(qt3d-opengl-renderer): include(opengl/opengl.pri)
