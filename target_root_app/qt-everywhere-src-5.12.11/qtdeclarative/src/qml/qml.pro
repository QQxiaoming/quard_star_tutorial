TARGET     = QtQml
QT = core-private

qtConfig(qml-network): \
    QT += network

TRACEPOINT_PROVIDER = $$PWD/qtqml.tracepoints
CONFIG += qt_tracepoints

DEFINES   += QT_NO_URL_CAST_FROM_STRING QT_NO_INTEGER_EVENT_COORDINATES

msvc:equals(QT_ARCH, i386): QMAKE_LFLAGS += /BASE:0x66000000
msvc:DEFINES *= _CRT_SECURE_NO_WARNINGS
win32:!winrt:LIBS += -lshell32
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2

# Ensure this gcc optimization is switched off for mips platforms to avoid trouble with JIT.
gcc:isEqual(QT_ARCH, "mips"): QMAKE_CXXFLAGS += -fno-reorder-blocks

DEFINES += QT_NO_FOREACH

!build_pass {
    # Create a header containing a hash that describes this library.  For a
    # released version of Qt, we'll use the .tag file that is updated by git
    # archive with the commit hash. For unreleased versions, we'll ask git
    # describe. Note that it won't update unless qmake is run again, even if
    # the commit change also changed something in this library.
    tagFile = $$PWD/../../.tag
    tag =
    exists($$tagFile) {
        tag = $$cat($$tagFile, singleline)
        QMAKE_INTERNAL_INCLUDED_FILES += $$tagFile
    }
    !equals(tag, "$${LITERAL_DOLLAR}Format:%H$${LITERAL_DOLLAR}") {
        QML_COMPILE_HASH = $$tag
    } else:exists($$PWD/../../.git) {
        commit = $$system(git rev-parse HEAD)
        QML_COMPILE_HASH = $$commit
    }
    compile_hash_contents = \
        "// Generated file, DO NOT EDIT" \
        "$${LITERAL_HASH}define QML_COMPILE_HASH \"$$QML_COMPILE_HASH\"" \
        "$${LITERAL_HASH}define QML_COMPILE_HASH_LENGTH $$str_size($$QML_COMPILE_HASH)"
    write_file("$$OUT_PWD/qml_compile_hash_p.h", compile_hash_contents)|error()
}

exists("qqml_enable_gcov") {
    QMAKE_CXXFLAGS = -fprofile-arcs -ftest-coverage -fno-elide-constructors
    LIBS_PRIVATE += -lgcov
}

# QTBUG-55238, disable new optimizer for MSVC 2015/Update 3.
release:msvc:equals(QT_CL_MAJOR_VERSION, 19):equals(QT_CL_MINOR_VERSION, 00): \
        greaterThan(QT_CL_PATCH_VERSION, 24212):QMAKE_CXXFLAGS += -d2SSAOptimizer-

QMAKE_DOCS = $$PWD/doc/qtqml.qdocconf

# 2415: variable "xx" of static storage duration was declared but never referenced
intel_icc: WERROR += -ww2415
# unused variable 'xx' [-Werror,-Wunused-const-variable]
greaterThan(QT_CLANG_MAJOR_VERSION, 3)|greaterThan(QT_CLANG_MINOR_VERSION, 3)| \
        greaterThan(QT_APPLE_CLANG_MAJOR_VERSION, 5)| \
        if(equals(QT_APPLE_CLANG_MAJOR_VERSION, 5):greaterThan(QT_APPLE_CLANG_MINOR_VERSION, 0)): \
    WERROR += -Wno-error=unused-const-variable

HEADERS += qtqmlglobal.h \
           qtqmlglobal_p.h

#modules
include(util/util.pri)
include(memory/memory.pri)
include(parser/parser.pri)
include(compiler/compiler.pri)
include(jsapi/jsapi.pri)
include(jsruntime/jsruntime.pri)
include(jit/jit.pri)
include(qml/qml.pri)
include(debugger/debugger.pri)
qtConfig(qml-animation) {
    include(animations/animations.pri)
}
include(types/types.pri)
include(../3rdparty/masm/masm-defs.pri)
include(../3rdparty/masm/masm.pri)

MODULE_PLUGIN_TYPES = \
    qmltooling
load(qt_module)
