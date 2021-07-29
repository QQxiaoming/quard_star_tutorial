INCLUDEPATH += $$PWD

win32: DEFINES += _CRT_SECURE_NO_WARNINGS

HEADERS += \
    $$PWD/dscameraservice.h \
    $$PWD/dscameracontrol.h \
    $$PWD/dsvideorenderer.h \
    $$PWD/dsvideodevicecontrol.h \
    $$PWD/dsimagecapturecontrol.h \
    $$PWD/dscamerasession.h \
    $$PWD/directshowcameraglobal.h \
    $$PWD/dscameraviewfindersettingscontrol.h \
    $$PWD/dscameraimageprocessingcontrol.h \
    $$PWD/directshowcameraexposurecontrol.h \
    $$PWD/directshowcameracapturedestinationcontrol.h \
    $$PWD/directshowcameracapturebufferformatcontrol.h \
    $$PWD/directshowcamerazoomcontrol.h \
    $$PWD/directshowcameraimageencodercontrol.h

SOURCES += \
    $$PWD/dscameraservice.cpp \
    $$PWD/dscameracontrol.cpp \
    $$PWD/dsvideorenderer.cpp \
    $$PWD/dsvideodevicecontrol.cpp \
    $$PWD/dsimagecapturecontrol.cpp \
    $$PWD/dscamerasession.cpp \
    $$PWD/dscameraviewfindersettingscontrol.cpp \
    $$PWD/dscameraimageprocessingcontrol.cpp \
    $$PWD/directshowcameraexposurecontrol.cpp \
    $$PWD/directshowcameracapturedestinationcontrol.cpp \
    $$PWD/directshowcameracapturebufferformatcontrol.cpp \
    $$PWD/directshowcamerazoomcontrol.cpp \
    $$PWD/directshowcameraimageencodercontrol.cpp

*-msvc*:INCLUDEPATH += $$(DXSDK_DIR)/include
QMAKE_USE += directshow
