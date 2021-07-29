TEMPLATE = subdirs

qtConfig(private_tests) {
    SUBDIRS += \
        entity \
        renderpass \
        qgraphicsutils \
        shader \
        shaderbuilder \
        texture \
        material \
        vsyncframeadvanceservice \
        meshfunctors \
        qattribute \
        qbuffer \
        qgeometry \
        qgeometryrenderer \
        qlevelofdetail \
        levelofdetail \
        buffer \
        attribute \
        geometry \
        geometryrenderer \
        qcameraselector \
        qclearbuffers \
        qframegraphnode \
        qlayerfilter \
        qabstractlight \
        qenvironmentlight \
        qrenderpassfilter \
        qrenderstate \
        qrendertargetselector \
        qsortpolicy \
        qrenderstateset \
        qtechniquefilter \
        qtextureimagedata \
        qviewport \
        framegraphnode \
        qobjectpicker \
        objectpicker \
#        qboundingvolumedebug \
#        boundingvolumedebug \
        ddstextures \
        shadercache \
        layerfiltering \
        filterentitybycomponent \
        genericlambdajob \
        qgraphicsapifilter \
        qrendersurfaceselector \
        sortpolicy \
        sceneloader \
        qsceneloader \
        qrendertargetoutput \
        qcameralens \
        qcomputecommand \
        loadscenejob \
        qrendercapture \
        uniform \
        qparameter \
        parameter \
        qtextureloader \
        qtextureimage \
        qabstracttexture \
        qabstracttextureimage \
        qrendersettings \
        texturedatamanager \
        rendertarget \
        transform \
        computecommand \
        qrendertarget \
        qdispatchcompute \
        qtechnique \
        qeffect \
        qrenderpass \
        qfilterkey \
        effect \
        filterkey \
        qmesh \
        technique \
        rendercapture \
        segmentvisitor \
        trianglevisitor \
        qmemorybarrier \
        memorybarrier \
        qshaderprogram \
        qshaderprogrambuilder \
        coordinatereader \
        framegraphvisitor \
        armature \
        skeleton \
        joint \
        qproximityfilter \
        proximityfilter \
        proximityfiltering \
        qblitframebuffer \
        blitframebuffer \
        qraycaster \
        raycaster \
        qscreenraycaster \
        raycastingjob \
        qcamera

    QT_FOR_CONFIG = 3dcore-private
    # TO DO: These could be restored to be executed in all cases
    # when aligned-malloc.pri becomes part of the test framework
    !qtConfig(qt3d-simd-avx2): {
      SUBDIRS += \
        qray3d \
        raycasting \
        trianglesextractor \
        triangleboundingvolume \
    }
}

# Tests related to the OpenGL renderer
QT_FOR_CONFIG += 3drender-private

qtConfig(qt3d-opengl-renderer):qtConfig(private_tests) {

    SUBDIRS += \
        filtercompatibletechniquejob \
        graphicshelpergl3_3 \
        graphicshelpergl3_2 \
        graphicshelpergl2 \
        materialparametergathererjob \
        textures \
        renderer \
        renderviewutils \
        renderviews \
        renderqueue \
        renderviewbuilder \
        sendrendercapturejob

    qtConfig(qt3d-extras) {
        SUBDIRS += \
            qmaterial \
            geometryloaders \
            picking \
            boundingsphere \
            qdefaultmeshes \
            pickboundingvolumejob \
            gltfplugins \
            updatemeshtrianglelistjob \
            updateshaderdatatransformjob
    }

    qtConfig(qt3d-input) {
        SUBDIRS += \
            qscene2d \
            scene2d
    }

    !macos: SUBDIRS += graphicshelpergl4

    qtConfig(qt3d-simd-avx2): SUBDIRS += alignedresourcesmanagers-avx
    qtConfig(qt3d-simd-sse2): SUBDIRS += alignedresourcesmanagers-sse
}
