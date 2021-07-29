TARGET = tst_alignedresourcesmanagers-avx
CONFIG += testcase simd
TEMPLATE = app

SOURCES += tst_alignedresourcesmanagers-avx.cpp

QT += testlib 3dcore 3dcore-private 3drender 3drender-private

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_AVX2
