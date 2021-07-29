TEMPLATE = subdirs
SUBDIRS =   quick-accessibility \
            animation \
            draganddrop \
            externaldraganddrop \
            canvas \
            imageelements \
            keyinteraction \
            layouts \
            localstorage \
            models \
            views \
            tableview \
            mousearea \
            positioners \
            righttoleft \
            scenegraph \
            shadereffects \
            text \
            threading \
            touchinteraction \
            tutorials \
            customitems \
            imageprovider \
            imageresponseprovider \
            window \
            particles \
            delegatechooser \
            shapes

#OpenGL Support Required
qtConfig(opengl(es1|es2)?) {
    SUBDIRS += \
    textureprovider \
    rendercontrol
}

# Widget dependent examples
qtHaveModule(widgets) {
    SUBDIRS += embeddedinwidgets
    qtHaveModule(quickwidgets):qtConfig(opengl(es1|es2)?): SUBDIRS += quickwidgets
}

EXAMPLE_FILES = \
    shared
