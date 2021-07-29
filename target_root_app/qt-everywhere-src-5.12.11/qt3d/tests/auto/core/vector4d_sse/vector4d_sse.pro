TARGET = tst_vector4d_sse
CONFIG += testcase simd

SOURCES += \
    tst_vector4d_sse.cpp

QT += testlib 3dcore 3dcore-private

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_SSE2
