TEMPLATE = subdirs

SUBDIRS = \
    handle \
    qresourcemanager \
    qcircularbuffer \
    qboundedcircularbuffer \
    nodes \
    qaspectengine \
    qaspectfactory \
    qaspectjob \
    qchangearbiter \
    qscene \
    qservicelocator \
    qjoint \
    qskeletonloader \
    qskeleton \
    qarmature

qtConfig(private_tests) {
    SUBDIRS += \
        qentity \
        qframeallocator \
        qtransform \
        threadpooler \
        qpostman \
        vector4d_base \
        vector3d_base

        QT_FOR_CONFIG += 3dcore-private
        qtConfig(qt3d-profile-jobs): SUBDIRS += aspectcommanddebugger
        qtConfig(qt3d-simd-sse2) {
            SUBDIRS += \
                vector4d_sse \
                vector3d_sse \
                matrix4x4_sse
        }
        qtConfig(qt3d-simd-avx2): SUBDIRS += matrix4x4_avx2
}
