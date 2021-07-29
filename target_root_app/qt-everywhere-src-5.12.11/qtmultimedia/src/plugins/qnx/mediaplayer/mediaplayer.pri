INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/mmrendereraudiorolecontrol.h \
    $$PWD/mmrenderermediaplayercontrol.h \
    $$PWD/mmrenderermediaplayerservice.h \
    $$PWD/mmrenderermetadata.h \
    $$PWD/mmrenderermetadatareadercontrol.h \
    $$PWD/mmrendererplayervideorenderercontrol.h \
    $$PWD/mmrendererutil.h \
    $$PWD/mmrenderervideowindowcontrol.h \
    $$PWD/mmreventmediaplayercontrol.h \
    $$PWD/mmreventthread.h \
    $$PWD/mmrenderercustomaudiorolecontrol.h
SOURCES += \
    $$PWD/mmrendereraudiorolecontrol.cpp \
    $$PWD/mmrenderermediaplayercontrol.cpp \
    $$PWD/mmrenderermediaplayerservice.cpp \
    $$PWD/mmrenderermetadata.cpp \
    $$PWD/mmrenderermetadatareadercontrol.cpp \
    $$PWD/mmrendererplayervideorenderercontrol.cpp \
    $$PWD/mmrendererutil.cpp \
    $$PWD/mmrenderervideowindowcontrol.cpp \
    $$PWD/mmreventmediaplayercontrol.cpp \
    $$PWD/mmreventthread.cpp \
    $$PWD/mmrenderercustomaudiorolecontrol.cpp

QMAKE_USE += mmrenderer
