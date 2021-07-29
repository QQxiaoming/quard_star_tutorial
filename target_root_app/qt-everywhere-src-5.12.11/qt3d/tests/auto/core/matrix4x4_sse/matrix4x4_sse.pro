TARGET = tst_matrix4x4_sse
CONFIG += testcase
QT += testlib 3dcore 3dcore-private

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_SSE2

SOURCES += tst_matrix4x4_sse.cpp

