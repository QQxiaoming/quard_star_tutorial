TEMPLATE = subdirs

SUBDIRS += \
    qanimationaspect \
    qanimationcliploader \
    qclipanimator \
    qblendedclipanimator \
    qchannelmapper \
    qclipblendvalue \
    qanimationcontroller \
    qanimationgroup \
    qkeyframeanimation \
    qmorphinganimation \
    qmorphtarget \
    qvertexblendanimation \
    qclock \
    qskeletonmapping \
    qcallbackmapping

qtConfig(private_tests) {
    SUBDIRS += \
        animationclip \
        fcurve \
        functionrangefinder \
        bezierevaluator \
        clipanimator \
        blendedclipanimator \
        channelmapper \
        channelmapping \
        qlerpclipblend \
        clipblendnodemanager \
        clipblendnode \
        lerpclipblend \
        clipblendnodevisitor \
        qadditiveclipblend \
        additiveclipblend \
        clipblendvalue \
        animationutils \
        qabstractanimation \
        clock \
        skeleton \
        findrunningclipanimatorsjob \
        updatepropertymapjob \
        qchannelmapping
}
