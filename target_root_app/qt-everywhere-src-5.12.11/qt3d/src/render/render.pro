TARGET   = Qt3DRender
MODULE   = 3drender

QT = core-private gui-private 3dcore-private
QT_PRIVATE = openglextensions
QT_FOR_PRIVATE = concurrent

include (backend/render-backend.pri)
include (geometry/geometry.pri)
include (framegraph/framegraph.pri)
include (frontend/render-frontend.pri)
include (jobs/jobs.pri)
include (lights/lights.pri)
include (materialsystem/materialsystem.pri)
include (renderstates/renderstates.pri)
include (io/io.pri)
include (picking/picking.pri)
include (raycasting/raycasting.pri)
include (services/services.pri)
include (texture/texture.pri)
include (renderers/renderers.pri)

# Qt3D is free of Q_FOREACH - make sure it stays that way:
DEFINES += QT_NO_FOREACH

gcov {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
}

qtConfig(qt3d-simd-avx2) {
    CONFIG += simd
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_AVX2
}

qtConfig(qt3d-simd-sse2):!qtConfig(qt3d-simd-avx2) {
    CONFIG += simd
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_SSE2
}

HEADERS += \
    renderlogging_p.h \
    qt3drender_global.h \
    qt3drender_global_p.h \
    aligned_malloc_p.h

SOURCES += \
    renderlogging.cpp

MODULE_PLUGIN_TYPES = \
    sceneparsers \
    geometryloaders \
    renderplugins

load(qt_module)
