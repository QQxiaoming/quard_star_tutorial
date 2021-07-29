SOURCES += \
    $$PWD/qtransform.cpp \
    $$PWD/qjoint.cpp \
    $$PWD/qabstractskeleton.cpp \
    $$PWD/qskeleton.cpp \
    $$PWD/qskeletonloader.cpp \
    $$PWD/qarmature.cpp

HEADERS += \
    $$PWD/qtransform.h \
    $$PWD/qtransform_p.h \
    $$PWD/qmath3d_p.h \
    $$PWD/qjoint.h \
    $$PWD/qjoint_p.h \
    $$PWD/qabstractskeleton.h \
    $$PWD/qabstractskeleton_p.h \
    $$PWD/qskeleton.h \
    $$PWD/qskeleton_p.h \
    $$PWD/qskeletonloader.h \
    $$PWD/qskeletonloader_p.h \
    $$PWD/qarmature.h \
    $$PWD/qarmature_p.h \
    $$PWD/vector4d_p.h \
    $$PWD/vector3d_p.h \
    $$PWD/matrix4x4_p.h \
    $$PWD/sqt_p.h

INCLUDEPATH += $$PWD

qtConfig(qt3d-simd-sse2) {
    CONFIG += simd

    SSE2_HEADERS += \
            $$PWD/matrix4x4_sse_p.h

    SSE2_SOURCES += \
            $$PWD/matrix4x4_sse.cpp

    # These files contain AVX2 code, only add them to SSE2 if AVX2 not available
    !qtConfig(qt3d-simd-avx2) {
        SSE2_SOURCES += \
            $$PWD/vector4d_sse.cpp \
            $$PWD/vector3d_sse.cpp
        SSE2_HEADERS += \
            $$PWD/vector4d_sse_p.h \
            $$PWD/vector3d_sse_p.h
    }
}

qtConfig(qt3d-simd-avx2) {
    CONFIG += simd

    AVX2_HEADERS += \
            $$PWD/matrix4x4_avx2_p.h \
            $$PWD/vector4d_sse_p.h \
            $$PWD/vector3d_sse_p.h
    AVX2_SOURCES += \
            $$PWD/matrix4x4_avx2.cpp \
            $$PWD/vector4d_sse.cpp \
            $$PWD/vector3d_sse.cpp
}
