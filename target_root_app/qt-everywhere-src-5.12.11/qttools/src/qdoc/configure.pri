defineReplace(extractVersion)      { return($$replace(1, ^(\\d+\\.\\d+\\.\\d+)(svn)?$, \\1)) }
defineReplace(extractMajorVersion) { return($$replace(1, ^(\\d+)\\.\\d+\\.\\d+(svn)?$, \\1)) }
defineReplace(extractMinorVersion) { return($$replace(1, ^\\d+\\.(\\d+)\\.\\d+(svn)?$, \\1)) }
defineReplace(extractPatchVersion) { return($$replace(1, ^\\d+\\.\\d+\\.(\\d+)(svn)?$, \\1)) }

defineTest(versionIsAtLeast) {
    actual_major_version = $$extractMajorVersion($$1)
    actual_minor_version = $$extractMinorVersion($$1)
    actual_patch_version = $$extractPatchVersion($$1)
    required_min_major_version = $$extractMajorVersion($$2)
    required_min_minor_version = $$extractMinorVersion($$2)
    required_min_patch_version = $$extractPatchVersion($$2)

    isEqual(actual_major_version, $$required_min_major_version) {
        isEqual(actual_minor_version, $$required_min_minor_version) {
            isEqual(actual_patch_version, $$required_min_patch_version): return(true)
            greaterThan(actual_patch_version, $$required_min_patch_version): return(true)
        }
        greaterThan(actual_minor_version, $$required_min_minor_version): return(true)
    }
    greaterThan(actual_major_version, $$required_min_major_version): return(true)

    return(false)
}

defineReplace(findLLVMVersionFromLibDir) {
    libdir = $$1
    version_dirs = $$files($$libdir/clang/*)
    for (version_dir, version_dirs) {
        fileName = $$basename(version_dir)
        version = $$find(fileName, ^(\\d+\\.\\d+\\.\\d+)$)
        !isEmpty(version) {
            isEmpty(candidateVersion): candidateVersion = $$version
            else: versionIsAtLeast($$version, $$candidateVersion): candidateVersion = $$version
        }
    }
    return($$candidateVersion)
}

defineTest(qtConfTest_libclang) {
    isEmpty(QDOC_USE_STATIC_LIBCLANG): QDOC_USE_STATIC_LIBCLANG = $$(QDOC_USE_STATIC_LIBCLANG)

    equals(QMAKE_HOST.os, Windows) {
        # on Windows we have only two host compilers, MSVC or mingw. The former we never
        # use for cross-compilation where it isn't also the target compiler. The latter
        # is not detectable as this .prf file is evaluated against the target configuration
        # and therefore checking for "mingw" won't work when the target compiler is clang (Android)
        # or qcc (QNX).
        msvc {
            isEmpty(LLVM_INSTALL_DIR): LLVM_INSTALL_DIR = $$(LLVM_INSTALL_DIR_MSVC)
        } else {
            isEmpty(LLVM_INSTALL_DIR): LLVM_INSTALL_DIR = $$(LLVM_INSTALL_DIR_MINGW)
        }
    }
    isEmpty(LLVM_INSTALL_DIR): LLVM_INSTALL_DIR = $$(LLVM_INSTALL_DIR)

    # Assume libclang is installed on the target system
    isEmpty(LLVM_INSTALL_DIR) {
        llvmConfigCandidates = \
            llvm-config-9 \
            llvm-config-8 \
            llvm-config-7 \
            llvm-config-6.0 \
            llvm-config-5.0 \
            llvm-config-4.0 \
            llvm-config-3.9 \
            llvm-config

        for (candidate, llvmConfigCandidates) {
            LLVM_INSTALL_DIR = $$system("$$candidate --prefix 2>$$QMAKE_SYSTEM_NULL_DEVICE")
            !isEmpty(LLVM_INSTALL_DIR) {
                CLANG_INCLUDEPATH = $$system("$$candidate --includedir 2>/dev/null")
                LIBCLANG_MAIN_HEADER = $$CLANG_INCLUDEPATH/clang-c/Index.h
                !exists($$LIBCLANG_MAIN_HEADER) {
                    !isEmpty(LLVM_INSTALL_DIR): \
                        qtLog("Cannot find libclang's main header file, candidate: $${LIBCLANG_MAIN_HEADER}.")
                        continue
                } else {
                    qtLog("Using Clang installation found in $${LLVM_INSTALL_DIR}." \
                          "Set the LLVM_INSTALL_DIR environment variable to override.")
                    break()
                }
            }
        }
    }
    LLVM_INSTALL_DIR = $$clean_path($$LLVM_INSTALL_DIR)

    contains(QMAKE_HOST.arch, x86_64): \
        clangInstallDir = $$replace(LLVM_INSTALL_DIR, _ARCH_, 64)
    else: \
        clangInstallDir = $$replace(LLVM_INSTALL_DIR, _ARCH_, 32)
    isEmpty(LLVM_INSTALL_DIR) {
        win32 {
            return(false)
        }
        macos {
            # Default to homebrew llvm on macOS. The CLANG_VERSION test below will complain if missing.
            clangInstallDir = $$system("brew --prefix llvm")
        } else {
            clangInstallDir = /usr
        }
    }

    # note: llvm_config only exits on unix
    llvm_config = $$clangInstallDir/bin/llvm-config
    exists($$llvm_config) {
        CLANG_LIBDIR = $$system("$$llvm_config --libdir 2>/dev/null")
        CLANG_INCLUDEPATH = $$system("$$llvm_config --includedir 2>/dev/null")
        output = $$system("$$llvm_config --version 2>/dev/null")
        CLANG_VERSION = $$extractVersion($$output)
    } else {
        CLANG_LIBDIR = $$clangInstallDir/lib
        CLANG_INCLUDEPATH = $$clangInstallDir/include
        CLANG_VERSION = $$findLLVMVersionFromLibDir($$CLANG_LIBDIR)
    }
    isEmpty(CLANG_VERSION) {
        !isEmpty(LLVM_INSTALL_DIR): \
            qtLog("Cannot determine version of clang installation in $${clangInstallDir}.")
        return(false)
    }

    LIBCLANG_MAIN_HEADER = $$CLANG_INCLUDEPATH/clang-c/Index.h
    !exists($$LIBCLANG_MAIN_HEADER) {
        !isEmpty(LLVM_INSTALL_DIR): \
            qtLog("Cannot find libclang's main header file, candidate: $${LIBCLANG_MAIN_HEADER}.")
        return(false)
    }

    !contains(QMAKE_DEFAULT_LIBDIRS, $$CLANG_LIBDIR): CLANG_LIBS = -L$${CLANG_LIBDIR}

    CLANG_DEFINES =

    isEmpty(QDOC_USE_STATIC_LIBCLANG) {
        equals(QMAKE_HOST.os, Windows): \
            CLANG_LIBS += -llibclang -ladvapi32 -lshell32
        else: \
            CLANG_LIBS += -lclang
    } else {
        msvc {
            CLANG_DEFINES += CINDEX_LINKAGE=
            CLANG_LIBS += -llibclang_static -ladvapi32 -lshell32 -lMincore
        } else {
            !equals(QMAKE_HOST.os, Darwin): CLANG_LIBS+=-Wl,--start-group
            CLANG_LIBS += -lclangAnalysis \
                        -lclangApplyReplacements \
                        -lclangARCMigrate \
                        -lclangAST \
                        -lclangASTMatchers \
                        -lclangBasic \
                        -lclangChangeNamespace \
                        -lclangCodeGen \
                        -lclangCrossTU \
                        -lclangDaemon \
                        -lclangDriver \
                        -lclangDynamicASTMatchers \
                        -lclangEdit \
                        -lclangFormat \
                        -lclangFrontend \
                        -lclangFrontendTool \
                        -lclangHandleCXX \
                        -lclangIncludeFixer \
                        -lclangIncludeFixerPlugin \
                        -lclangIndex \
                        -lclangLex \
                        -lclangMove \
                        -lclangParse \
                        -lclangQuery \
                        -lclangReorderFields \
                        -lclangRewrite \
                        -lclangRewriteFrontend \
                        -lclangSema \
                        -lclangSerialization \
                        -lclang_static \
                        -lclangStaticAnalyzerCheckers \
                        -lclangStaticAnalyzerCore \
                        -lclangStaticAnalyzerFrontend \
                        -lclangTidy \
                        -lclangTidyAndroidModule \
                        -lclangTidyBoostModule \
                        -lclangTidyBugproneModule \
                        -lclangTidyCERTModule \
                        -lclangTidyCppCoreGuidelinesModule \
                        -lclangTidyFuchsiaModule \
                        -lclangTidyGoogleModule \
                        -lclangTidyHICPPModule \
                        -lclangTidyLLVMModule \
                        -lclangTidyMiscModule \
                        -lclangTidyModernizeModule \
                        -lclangTidyMPIModule \
                        -lclangTidyObjCModule \
                        -lclangTidyPerformanceModule \
                        -lclangTidyPlugin \
                        -lclangTidyReadabilityModule \
                        -lclangTidyUtils \
                        -lclangTooling \
                        -lclangToolingASTDiff \
                        -lclangToolingCore \
                        -lclangToolingRefactor \
                        -lfindAllSymbols \
                        -lLLVMAArch64AsmParser \
                        -lLLVMAArch64AsmPrinter \
                        -lLLVMAArch64CodeGen \
                        -lLLVMAArch64Desc \
                        -lLLVMAArch64Disassembler \
                        -lLLVMAArch64Info \
                        -lLLVMAArch64Utils \
                        -lLLVMAMDGPUAsmParser \
                        -lLLVMAMDGPUAsmPrinter \
                        -lLLVMAMDGPUCodeGen \
                        -lLLVMAMDGPUDesc \
                        -lLLVMAMDGPUDisassembler \
                        -lLLVMAMDGPUInfo \
                        -lLLVMAMDGPUUtils \
                        -lLLVMAnalysis \
                        -lLLVMARMAsmParser \
                        -lLLVMARMAsmPrinter \
                        -lLLVMARMCodeGen \
                        -lLLVMARMDesc \
                        -lLLVMARMDisassembler \
                        -lLLVMARMInfo \
                        -lLLVMARMUtils \
                        -lLLVMAsmParser \
                        -lLLVMAsmPrinter \
                        -lLLVMBinaryFormat \
                        -lLLVMBitReader \
                        -lLLVMBitWriter \
                        -lLLVMBPFAsmParser \
                        -lLLVMBPFAsmPrinter \
                        -lLLVMBPFCodeGen \
                        -lLLVMBPFDesc \
                        -lLLVMBPFDisassembler \
                        -lLLVMBPFInfo \
                        -lLLVMCodeGen \
                        -lLLVMCore \
                        -lLLVMCoroutines \
                        -lLLVMCoverage \
                        -lLLVMDebugInfoCodeView \
                        -lLLVMDebugInfoDWARF \
                        -lLLVMDebugInfoMSF \
                        -lLLVMDebugInfoPDB \
                        -lLLVMDemangle \
                        -lLLVMDlltoolDriver \
                        -lLLVMExecutionEngine \
                        -lLLVMFuzzMutate \
                        -lLLVMGlobalISel \
                        -lLLVMHexagonAsmParser \
                        -lLLVMHexagonCodeGen \
                        -lLLVMHexagonDesc \
                        -lLLVMHexagonDisassembler \
                        -lLLVMHexagonInfo \
                        -lLLVMInstCombine \
                        -lLLVMInstrumentation \
                        -lLLVMInterpreter \
                        -lLLVMipo \
                        -lLLVMIRReader \
                        -lLLVMLanaiAsmParser \
                        -lLLVMLanaiAsmPrinter \
                        -lLLVMLanaiCodeGen \
                        -lLLVMLanaiDesc \
                        -lLLVMLanaiDisassembler \
                        -lLLVMLanaiInfo \
                        -lLLVMLibDriver \
                        -lLLVMLineEditor \
                        -lLLVMLinker \
                        -lLLVMLTO \
                        -lLLVMMC \
                        -lLLVMMCDisassembler \
                        -lLLVMMCJIT \
                        -lLLVMMCParser \
                        -lLLVMMipsAsmParser \
                        -lLLVMMipsAsmPrinter \
                        -lLLVMMipsCodeGen \
                        -lLLVMMipsDesc \
                        -lLLVMMipsDisassembler \
                        -lLLVMMipsInfo \
                        -lLLVMMIRParser \
                        -lLLVMMSP430AsmPrinter \
                        -lLLVMMSP430CodeGen \
                        -lLLVMMSP430Desc \
                        -lLLVMMSP430Info \
                        -lLLVMNVPTXAsmPrinter \
                        -lLLVMNVPTXCodeGen \
                        -lLLVMNVPTXDesc \
                        -lLLVMNVPTXInfo \
                        -lLLVMObjCARCOpts \
                        -lLLVMObject \
                        -lLLVMObjectYAML \
                        -lLLVMOption \
                        -lLLVMOrcJIT \
                        -lLLVMPasses \
                        -lLLVMPowerPCAsmParser \
                        -lLLVMPowerPCAsmPrinter \
                        -lLLVMPowerPCCodeGen \
                        -lLLVMPowerPCDesc \
                        -lLLVMPowerPCDisassembler \
                        -lLLVMPowerPCInfo \
                        -lLLVMProfileData \
                        -lLLVMRuntimeDyld \
                        -lLLVMScalarOpts \
                        -lLLVMSelectionDAG \
                        -lLLVMSparcAsmParser \
                        -lLLVMSparcAsmPrinter \
                        -lLLVMSparcCodeGen \
                        -lLLVMSparcDesc \
                        -lLLVMSparcDisassembler \
                        -lLLVMSparcInfo \
                        -lLLVMSupport \
                        -lLLVMSymbolize \
                        -lLLVMSystemZAsmParser \
                        -lLLVMSystemZAsmPrinter \
                        -lLLVMSystemZCodeGen \
                        -lLLVMSystemZDesc \
                        -lLLVMSystemZDisassembler \
                        -lLLVMSystemZInfo \
                        -lLLVMTableGen \
                        -lLLVMTarget \
                        -lLLVMTransformUtils \
                        -lLLVMVectorize \
                        -lLLVMWindowsManifest \
                        -lLLVMX86AsmParser \
                        -lLLVMX86AsmPrinter \
                        -lLLVMX86CodeGen \
                        -lLLVMX86Desc \
                        -lLLVMX86Disassembler \
                        -lLLVMX86Info \
                        -lLLVMX86Utils \
                        -lLLVMXCoreAsmPrinter \
                        -lLLVMXCoreCodeGen \
                        -lLLVMXCoreDesc \
                        -lLLVMXCoreDisassembler \
                        -lLLVMXCoreInfo \
                        -lLLVMXRay
            !equals(QMAKE_HOST.os, Darwin): CLANG_LIBS+=-Wl,--end-group
            CLANG_LIBS += -lz
            equals(QMAKE_HOST.os, Windows): CLANG_LIBS += -lpsapi -lshell32 -lole32 -luuid -ladvapi32 -lversion
            else: CLANG_LIBS += -ldl
            equals(QMAKE_HOST.os, Darwin): CLANG_LIBS += -lcurses -lm -lxml2
        }
    }

    !versionIsAtLeast($$CLANG_VERSION, "3.9.0") {
        log("LLVM/Clang version >= 3.9.0 required, version provided: $${CLANG_VERSION}.$$escape_expand(\\n)")
        return(false)
    }

    $${1}.libs = $$CLANG_LIBS
    export($${1}.libs)
    $${1}.cache += libs

    $${1}.includepath = $$CLANG_INCLUDEPATH
    export($${1}.includepath)
    $${1}.cache += includepath

    $${1}.libdir = $$CLANG_LIBDIR
    export($${1}.libdir)
    $${1}.cache += libdir

    $${1}.defines = $$CLANG_DEFINES
    export($${1}.defines)
    $${1}.cache += defines

    $${1}.version = $$CLANG_VERSION
    export($${1}.version)
    $${1}.cache += version

    export($${1}.cache)

    return(true)
}

