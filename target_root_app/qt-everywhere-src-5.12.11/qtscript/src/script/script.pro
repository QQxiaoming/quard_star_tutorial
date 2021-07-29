TARGET     = QtScript
QT         = core-private

DEFINES   += JSC=QTJSC jscyyparse=qtjscyyparse jscyylex=qtjscyylex jscyyerror=qtjscyyerror WTF=QTWTF
DEFINES   += QT_NO_USING_NAMESPACE
DEFINES   += QLALR_NO_QSCRIPTGRAMMAR_DEBUG_INFO
#win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000       ### FIXME

QMAKE_DOCS = $$PWD/doc/qtscript.qdocconf

CONFIG += building-libs

WEBKITDIR = $$PWD/../3rdparty/javascriptcore
GENERATED_SOURCES_DIR = generated

include($$WEBKITDIR/WebKit.pri)

# Disable a few warnings on Windows.
# These are in addition to the ones disabled in WebKit.pri
win32-msvc*: QMAKE_CXXFLAGS += -wd4396 -wd4099 -wd4577

# Windows CE-specific stuff copied from WebCore.pro
# ### Should rather be in JavaScriptCore.pri?
wince* {
    INCLUDEPATH += $$WEBKITDIR/JavaScriptCore/os-wince
    INCLUDEPATH += $$WEBKITDIR/JavaScriptCore/os-win32
    LIBS_PRIVATE += -lmmtimer
}

mac {
    DEFINES += ENABLE_JSC_MULTIPLE_THREADS=0
    contains(QT_CONFIG, coreservices) {
      LIBS_PRIVATE += -framework CoreServices
    } else {
      LIBS_PRIVATE += -framework CoreFoundation
    }
}

qtConfig(asm_hwcap_h): DEFINES += HAVE_asm_hwcap_h

# Suppress 'LEAK' messages (see QTBUG-18201)
DEFINES += LOG_DISABLED=1

include($$WEBKITDIR/JavaScriptCore/JavaScriptCore.pri)

# This line copied from WebCore.pro
DEFINES += WTF_USE_JAVASCRIPTCORE_BINDINGS=1 WTF_CHANGES=1

# Avoid JSC C API functions being exported.
DEFINES += JS_NO_EXPORT

INCLUDEPATH += $$PWD

include(script.pri)

integrity {
    CFLAGS += --diag_remark=236,82
}

TR_EXCLUDE = $$WEBKITDIR/*

load(qt_module)
# Override the module default, as it breaks compilation with libc++>=5.0:
# auto_ptr is no longer part of the c++1z standard.
CONFIG -= c++1z

android-clang:equals(ANDROID_TARGET_ARCH, armeabi-v7a) {
    QMAKE_CFLAGS_RELEASE -= -mthumb
    QMAKE_CFLAGS_RELEASE *= -marm
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -= -mthumb
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO *= -marm
    QMAKE_CXXFLAGS_RELEASE -= -mthumb
    QMAKE_CXXFLAGS_RELEASE *= -marm
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -mthumb
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO *= -marm
}
