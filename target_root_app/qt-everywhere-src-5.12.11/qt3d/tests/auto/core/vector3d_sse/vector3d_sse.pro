TARGET = tst_vector3d_sse
CONFIG += testcase simd
QT += testlib 3dcore 3dcore-private

SOURCES += \
    tst_vector3d_sse.cpp

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_SSE2
