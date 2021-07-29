TEMPLATE      = subdirs
SUBDIRS      += comapp \
                hierarchy \
                menus \
                multiple \
                simple \
                wrapper

contains(QT_CONFIG, shared):SUBDIRS += mediaplayer
contains(QT_CONFIG, opengl):!contains(QT_CONFIG, opengles2): SUBDIRS += opengl
qtHaveModule(quickcontrols2):SUBDIRS += simpleqml

# Other examples need an IDL compiler
mingw:SUBDIRS = mediaplayer
