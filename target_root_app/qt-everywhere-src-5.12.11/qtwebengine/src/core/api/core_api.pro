TARGET = qtwebenginecoreapi$$qtPlatformTargetSuffix()
DESTDIR = $$OUT_PWD/$$getConfigDir()

TEMPLATE = lib

CONFIG += staticlib
QT += network core-private webenginecoreheaders-private

# Don't create .prl file for this intermediate library because
# their contents get used when linking against them, breaking
# "-Wl,-whole-archive -lqtwebenginecoreapi --Wl,-no-whole-archive"
CONFIG -= create_prl

# Copy this logic from qt_module.prf so that the intermediate library can be
# created to the same rules as the final module linking in core_module.pro.
!host_build:if(win32|mac):!macx-xcode {
    qtConfig(debug_and_release): CONFIG += debug_and_release build_all
}

DEFINES += \
    BUILDING_CHROMIUM \
    NOMINMAX

CHROMIUM_SRC_DIR = $$QTWEBENGINE_ROOT/$$getChromiumSrcDir()
CHROMIUM_GEN_DIR = $$OUT_PWD/../$$getConfigDir()/gen
INCLUDEPATH += $$QTWEBENGINE_ROOT/src/core \
               $$CHROMIUM_GEN_DIR \
               $$CHROMIUM_SRC_DIR

gcc: QMAKE_CXXFLAGS_WARN_ON = -Wno-unused-parameter

HEADERS = \
    qwebenginecallback.h \
    qwebenginecallback_p.h \
    qtwebenginecoreglobal.h \
    qtwebenginecoreglobal_p.h \
    qwebenginecookiestore.h \
    qwebenginecookiestore_p.h \
    qwebenginehttprequest.h \
    qwebenginemessagepumpscheduler_p.h \
    qwebenginequotarequest.h \
    qwebengineregisterprotocolhandlerrequest.h \
    qwebengineurlrequestinterceptor.h \
    qwebengineurlrequestinfo.h \
    qwebengineurlrequestinfo_p.h \
    qwebengineurlrequestjob.h \
    qwebengineurlscheme.h \
    qwebengineurlschemehandler.h

SOURCES = \
    qtwebenginecoreglobal.cpp \
    qwebenginecookiestore.cpp \
    qwebenginehttprequest.cpp \
    qwebenginemessagepumpscheduler.cpp \
    qwebenginequotarequest.cpp \
    qwebengineregisterprotocolhandlerrequest.cpp \
    qwebengineurlrequestinfo.cpp \
    qwebengineurlrequestjob.cpp \
    qwebengineurlscheme.cpp \
    qwebengineurlschemehandler.cpp

### Qt6 Remove this workaround
unix:!isEmpty(QMAKE_LFLAGS_VERSION_SCRIPT):!static {
    CONFIG -= warning_clean
    SOURCES += qtbug-60565.cpp \
               qtbug-61521.cpp
}

msvc {
    # Create a list of object files that can be used as response file for the linker.
    # This is done to simulate -whole-archive on MSVC.
    QMAKE_POST_LINK = \
        "if exist $(DESTDIR_TARGET).objects del $(DESTDIR_TARGET).objects$$escape_expand(\\n\\t)" \
        "for %%a in ($(OBJECTS)) do echo $$shell_quote($$shell_path($$OUT_PWD))\\%%a >> $(DESTDIR_TARGET).objects"
}

load(qt_common)
