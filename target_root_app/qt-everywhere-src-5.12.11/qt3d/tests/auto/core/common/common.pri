SOURCES += \
    $$PWD/testpostmanarbiter.cpp

HEADERS += \
    $$PWD/testpostmanarbiter.h

INCLUDEPATH += $$PWD

qtConfig(private_tests) {
    SOURCES += \
        $$PWD/qbackendnodetester.cpp

    HEADERS += \
        $$PWD/qbackendnodetester.h
}

QT += core-private 3dcore 3dcore-private

qtConfig(qt3d-simd-avx2) {
    CONFIG += simd
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_AVX2
}

qtConfig(qt3d-simd-sse2):!qtConfig(qt3d-simd-avx2) {
    CONFIG += simd
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_SSE2
}
