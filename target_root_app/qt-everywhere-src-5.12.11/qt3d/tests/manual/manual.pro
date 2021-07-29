TEMPLATE = subdirs

SUBDIRS += \
    assimp \
    bigscene-cpp \
    bigmodel-qml \
    bigscene-instanced-qml \
    clip-planes-qml \
    component-changes \
    custom-mesh-cpp \
    custom-mesh-cpp-indirect \
    custom-mesh-qml \
    custom-mesh-update-data-cpp \
    custom-mesh-update-data-qml \
    cylinder-cpp \
    cylinder-parent-test \
    cylinder-qml \
    deferred-renderer-cpp \
    deferred-renderer-qml \
    downloading \
    dragging \
    dynamicscene-cpp \
    enabled-qml \
    gltf \
    gooch-qml \
    keyboardinput-qml \
    loader-qml \
    lod \
    mouseinput-qml \
    multiplewindows-qml \
    picking-qml \
    plasma \
    pointlinesize \
    scene3d-loader \
    simple-shaders-qml \
    skybox \
    tessellation-modes \
    transforms-qml \
    spritegrid \
    transparency-qml \
    transparency-qml-scene3d \
    rendercapture-qml \
    additional-attributes-qml \
    dynamic-model-loader-qml \
    buffercapture-qml \
    render-qml-to-texture \
    render-qml-to-texture-qml \
    video-texture-qml \
    animation-keyframe-simple \
    animation-keyframe-blendtree \
    distancefieldtext \
    mesh-morphing \
    anim-viewer \
    animation-keyframe-programmatic \
    layerfilter-qml \
    skinned-mesh \
    rigged-simple \
    proximityfilter \
    rendercapture-qml-fbo \
    blitframebuffer-qml \
    raycasting-qml \
    shared_texture_image \
    texture_property_updates \
    qtbug-72236 \
    qtbug-76766

qtHaveModule(widgets): {
    SUBDIRS += \
        assimp-cpp \
        paintedtexture-cpp \
        rendercapture-cpp
}

qtHaveModule(quickwidgets): SUBDIRS += quickwidget-switch
