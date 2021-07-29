INCLUDEPATH += $$PWD
INCLUDEPATH += $$OUT_PWD

HEADERS += \
    $$PWD/qv4bytecodegenerator_p.h \
    $$PWD/qv4compileddata_p.h \
    $$PWD/qv4compiler_p.h \
    $$PWD/qv4compilercontext_p.h \
    $$PWD/qv4compilercontrolflow_p.h \
    $$PWD/qv4compilerscanfunctions_p.h \
    $$PWD/qv4codegen_p.h \
    $$PWD/qqmlirbuilder_p.h \
    $$PWD/qqmltypecompiler_p.h \
    $$PWD/qv4instr_moth_p.h \
    $$PWD/qv4bytecodehandler_p.h

SOURCES += \
    $$PWD/qv4bytecodegenerator.cpp \
    $$PWD/qv4compileddata.cpp \
    $$PWD/qv4compiler.cpp \
    $$PWD/qv4compilercontext.cpp \
    $$PWD/qv4compilerscanfunctions.cpp \
    $$PWD/qv4codegen.cpp \
    $$PWD/qqmlirbuilder.cpp \
    $$PWD/qv4instr_moth.cpp \
    $$PWD/qv4bytecodehandler.cpp

!qmldevtools_build {

HEADERS += \
    $$PWD/qqmltypecompiler_p.h \
    $$PWD/qqmlpropertycachecreator_p.h \
    $$PWD/qqmlpropertyvalidator_p.h \
    $$PWD/qv4compilationunitmapper_p.h


SOURCES += \
    $$PWD/qqmltypecompiler.cpp \
    $$PWD/qqmlpropertycachecreator.cpp \
    $$PWD/qqmlpropertyvalidator.cpp \
    $$PWD/qv4compilationunitmapper.cpp

unix: SOURCES += $$PWD/qv4compilationunitmapper_unix.cpp
else: SOURCES += $$PWD/qv4compilationunitmapper_win.cpp
}

gcc {
    equals(QT_GCC_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -fno-strict-aliasing
}
