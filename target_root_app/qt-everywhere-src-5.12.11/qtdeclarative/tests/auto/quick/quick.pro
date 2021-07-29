TEMPLATE = subdirs

PUBLICTESTS += \
    geometry \
    qquickpixmapcache

qtConfig(opengl(es1|es2)?) {
    PUBLICTESTS += \
        drawingmodes \
        rendernode
    qtHaveModule(widgets): PUBLICTESTS += nodes

    QUICKTESTS += \
        qquickanimatedsprite \
        qquickframebufferobject \
        qquickopenglinfo \
        qquickspritesequence \
        qquickshadereffect
}

!cross_compile: PRIVATETESTS += examples

# This test requires the qtconcurrent module
!qtHaveModule(concurrent): PUBLICTESTS -= qquickpixmapcache

PRIVATETESTS += \
    nokeywords \
    propertyrequirements \
    qquickanimations \
    qquickapplication \
    qquickbehaviors \
    qquickfontloader \
    qquickfontloader_static \
    qquickfontmetrics \
    qquickimageprovider \
    qquicklayouts \
    qquickpath \
    qquicksmoothedanimation \
    qquickspringanimation \
    qquickanimationcontroller \
    qquickstyledtext \
    qquickstates \
    qquicksystempalette \
    qquicktimeline

QUICKTESTS += \
    pointerhandlers \
    qquickaccessible \
    qquickanchors \
    qquickanimatedimage \
    qquickdynamicpropertyanimation \
    qquickborderimage \
    qquickwindow \
    qquickdrag \
    qquickdroparea \
    qquickflickable \
    qquickflipable \
    qquickfocusscope \
    qquickgraphicsinfo \
    qquickgridview \
    qquickimage \
    qquickitem \
    qquickitem2 \
    qquickitemlayer \
    qquicklistview \
    qquicktableview \
    qquickloader \
    qquickmousearea \
    qquickmultipointtoucharea \
    qquickpainteditem \
    qquickshape \
    qquickpathview \
    qquickpincharea \
    qquickpositioners \
    qquickrectangle \
    qquickrepeater \
    qquickshortcut \
    qquicktext \
    qquicktextdocument \
    qquicktextedit \
    qquicktextinput \
    qquickvisualdatamodel \
    qquickview \
    qquickcanvasitem \
    qquickdesignersupport \
    qquickscreen \
    touchmouse \
    scenegraph \
    sharedimage

SUBDIRS += $$PUBLICTESTS

# Following tests are too slow on qemu + software backend
boot2qt: QUICKTESTS -= qquickgridview qquicklistview qquicktableview qquickpositioners

!qtConfig(accessibility):QUICKTESTS -= qquickaccessible

qtConfig(private_tests) {
    SUBDIRS += $$PRIVATETESTS
    SUBDIRS += $$QUICKTESTS
}
