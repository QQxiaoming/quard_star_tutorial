include(src/core/config/functions.pri)

# this must be done outside any function
QTWEBENGINE_SOURCE_TREE = $$PWD

equals(QMAKE_HOST.os, Windows): EXE_SUFFIX = .exe

defineTest(isPythonVersionSupported) {
    python = $$system_quote($$system_path($$1))
    python_version = $$system('$$python -c "import sys; print(sys.version_info[0:3])"')
    python_version ~= s/[()]//g
    python_version = $$split(python_version, ',')
    python_major_version = $$first(python_version)
    greaterThan(python_major_version, 2) {
        qtLog("Python version 3 is not supported by Chromium.")
        return(false)
    }
    python_minor_version = $$member(python_version, 1)
    python_patch_version = $$member(python_version, 2)
    greaterThan(python_major_version, 1): greaterThan(python_minor_version, 6): greaterThan(python_patch_version, 4): return(true)
    qtLog("Unsupported python version: $${python_major_version}.$${python_minor_version}.$${python_patch_version}.")
    return(false)
}

defineTest(qtConfTest_detectPython2) {
    python = $$qtConfFindInPath("python2$$EXE_SUFFIX")
    isEmpty(python) {
        qtLog("'python2$$EXE_SUFFIX' not found in PATH. Checking for 'python$$EXE_SUFFIX'.")
        python = $$qtConfFindInPath("python$$EXE_SUFFIX")
    }
    isEmpty(python) {
        qtLog("'python$$EXE_SUFFIX' not found in PATH. Giving up.")
        return(false)
    }
    !isPythonVersionSupported($$python) {
        qtLog("A suitable Python 2 executable could not be located.")
        return(false)
    }

    # Make tests.python2.location available in configure.json.
    $${1}.location = $$clean_path($$python)
    export($${1}.location)
    $${1}.cache += location
    export($${1}.cache)

    return(true)
}

defineReplace(qtConfFindGnuTool) {
    equals(QMAKE_HOST.os, Windows) {
        gnuwin32bindir = $$absolute_path($$QTWEBENGINE_SOURCE_TREE/../gnuwin32/bin)
        gnuwin32toolpath = "$$gnuwin32bindir/$${1}"
        exists($$gnuwin32toolpath): \
            return($$gnuwin32toolpath)
    }
    return($$qtConfFindInPath($$1))
}

defineTest(qtConfTest_detectGperf) {
    gperf = $$qtConfFindGnuTool("gperf$$EXE_SUFFIX")
    isEmpty(gperf) {
        qtLog("Required gperf could not be found.")
        return(false)
    }
    qtLog("Found gperf from path: $$gperf")
    return(true)
}

defineTest(qtConfTest_detectBison) {
    bison = $$qtConfFindGnuTool("bison$$EXE_SUFFIX")
    isEmpty(bison) {
        qtLog("Required bison could not be found.")
        return(false)
    }
    qtLog("Found bison from path: $$bison")
    return(true)
}

defineTest(qtConfTest_detectFlex) {
    flex = $$qtConfFindGnuTool("flex$$EXE_SUFFIX")
    isEmpty(flex) {
        qtLog("Required flex could not be found.")
        return(false)
    }
    qtLog("Found flex from path: $$flex")
    return(true)
}

defineTest(qtConfTest_detectGlibc) {
    ldd = $$qtConfFindInPath("ldd")
    !isEmpty(ldd) {
        qtLog("Found ldd from path: $$ldd")
        qtRunLoggedCommand("$$ldd --version", version)|return(true)
        version ~= 's/^.*[^0-9]\([0-9]*\.[0-9]*\).*$/\1/'
        version = $$first(version)
        qtLog("Found libc version: $$version")
        version = $$split(version,'.')
        version = $$member(version, 1)
        greaterThan(version, 16) {
            return(true)
        }
        qtLog("Detected too old version of glibc. Required min 2.17.")
        return(false)
    }
    qtLog("No ldd found. Assuming right version of glibc.")
    return(true)
}

defineTest(qtConfTest_detectNinja) {
    ninja = $$qtConfFindInPath("ninja$$EXE_SUFFIX")
    !isEmpty(ninja) {
        qtLog("Found ninja from path: $$ninja")
        qtRunLoggedCommand("$$ninja --version", version)|return(false)
        contains(version, "1.[7-9].*"): return(true)
        qtLog("Ninja version too old")
    }
    qtLog("Building own ninja")
    return(false)
}

defineTest(qtConfTest_detectProtoc) {
    protoc = $$qtConfFindInPath("protoc")
    isEmpty(protoc) {
        qtLog("Optional protoc could not be found.")
        return(false)
    }
    qtLog("Found protoc from path: $$protoc")
    return(true)
}

defineTest(qtConfTest_detectGn) {
    gn = $$qtConfFindInPath("gn$$EXE_SUFFIX")
    !isEmpty(gn) {
        qtRunLoggedCommand("$$gn --version", version)|return(false)
        #accept all for now
        contains(version, ".*"): return(true)
        qtLog("Gn version too old")
    }
    qtLog("Building own gn")
    return(false)
}

defineTest(qtConfTest_embedded) {
    lessThan(QT_MINOR_VERSION, 9) {
        cross_compile: return(true)
        return(false)
    }
    $$qtConfEvaluate("features.cross_compile"): return(true)
    return(false)
}

defineTest(qtConfTest_detectHostPkgConfig) {
   PKG_CONFIG = $$qtConfPkgConfig(true)
   isEmpty(PKG_CONFIG) {
       qtLog("Could not find host pkg-config")
       return(false)
   }
   qtLog("Found host pkg-config: $$PKG_CONFIG")
   $${1}.path = $$PKG_CONFIG
   export($${1}.path)
   $${1}.cache += path
   export($${1}.cache)
   return(true)
}

defineTest(qtConfTest_isSanitizerSupported) {
  sanitizer_combo_supported = true

  sanitize_address {
    asan_supported = false
    linux-clang-libc++:isSanitizerSupportedOnLinux() {
      asan_supported = true
    } else:macos:isSanitizerSupportedOnMacOS() {
      asan_supported = true
    }
    !$$asan_supported {
      sanitizer_combo_supported = false
      qtLog("An address sanitizer-enabled Qt WebEngine build can only be built on Linux or macOS using Clang and libc++.")
    }
  }

  sanitize_memory {
    sanitizer_combo_supported = false
    qtLog("A memory sanitizer-enabled Qt WebEngine build is not supported.")
  }

  sanitize_undefined {
    ubsan_supported = false
    CONFIG(release, debug|release):!debug_and_release {
      linux-clang-libc++:isSanitizerSupportedOnLinux() {
        ubsan_supported = true
      } else:macos:isSanitizerSupportedOnMacOS() {
        ubsan_supported = true
      }
    }
    !$$ubsan_supported {
      sanitizer_combo_supported = false
      qtLog("An undefined behavior sanitizer-enabled Qt WebEngine build can only be built on Linux or macOS using Clang and libc++ in release mode.")
    }
  }

  sanitize_thread {
    tsan_supported = false
    linux-clang-libc++:isSanitizerSupportedOnLinux() {
      tsan_supported = true
    }
    !$$tsan_supported {
      sanitizer_combo_supported = false
      qtLog("A thread sanitizer-enabled Qt WebEngine build can only be built on Linux using Clang and libc++.")
    }
  }

  $$sanitizer_combo_supported: return(true)
  return(false)
}

defineTest(isSanitizerSupportedOnLinux) {
  isSanitizerLinuxClangVersionSupported(): return(true)
  return(false)
}

defineTest(isSanitizerSupportedOnMacOS) {
  isEmpty(QMAKE_APPLE_CLANG_MAJOR_VERSION) {
    QTWEBENGINE_CLANG_IS_APPLE = false
  } else {
    QTWEBENGINE_CLANG_IS_APPLE = true
  }

  $$QTWEBENGINE_CLANG_IS_APPLE:isSanitizerMacOSAppleClangVersionSupported(): return(true)
  else:isSanitizerMacOSClangVersionSupported(): return(true)
  return(false)
}

defineTest(isSanitizerMacOSAppleClangVersionSupported) {
  # Clang sanitizer suppression attributes work from Apple Clang version 7.3.0+.
  greaterThan(QMAKE_APPLE_CLANG_MAJOR_VERSION, 7): return(true)
  greaterThan(QMAKE_APPLE_CLANG_MINOR_VERSION, 2): return(true)

  qtLog("Using Apple Clang version $${QMAKE_APPLE_CLANG_MAJOR_VERSION}.$${QMAKE_APPLE_CLANG_MINOR_VERSION}.$${QMAKE_APPLE_CLANG_PATCH_VERSION}, but at least Apple Clang version 7.3.0 is required to build a sanitizer-enabled Qt WebEngine.")
  return(false)
}

defineTest(isSanitizerMacOSClangVersionSupported) {
  # Clang sanitizer suppression attributes work from non-apple Clang version 3.7+.
  greaterThan(QMAKE_CLANG_MAJOR_VERSION, 3): return(true)
  greaterThan(QMAKE_CLANG_MINOR_VERSION, 6): return(true)

  qtLog("Using Clang version $${QMAKE_CLANG_MAJOR_VERSION}.$${QMAKE_CLANG_MINOR_VERSION}, but at least Clang version 3.7 is required to build a sanitizer-enabled Qt WebEngine.")
  return(false)
}

defineTest(isSanitizerLinuxClangVersionSupported) {
  # Clang sanitizer suppression attributes work from Clang version 3.7+.
  greaterThan(QMAKE_CLANG_MAJOR_VERSION, 3): return(true)
  greaterThan(QMAKE_CLANG_MINOR_VERSION, 6): return(true)

  qtLog("Using Clang version $${QMAKE_CLANG_MAJOR_VERSION}.$${QMAKE_CLANG_MINOR_VERSION}, but at least Clang version 3.7 is required to build a sanitizer-enabled Qt WebEngine.")
  return(false)
}

defineReplace(qtConfFunc_isTestsInBuildParts) {
    contains(QT_BUILD_PARTS, tests): return(true)
    return(false)
}

defineReplace(webEngineGetMacOSVersion) {
    value = $$system("sw_vers -productVersion 2>/dev/null")
    return($$value)
}

defineReplace(webEngineGetMacOSSDKVersion) {
    value = $$system("/usr/bin/xcodebuild -sdk $$QMAKE_MAC_SDK -version ProductVersion 2>/dev/null")
    return($$value)
}

defineReplace(webEngineGetMacOSClangVerboseVersion) {
    output = $$system("$$QMAKE_CXX --version 2>/dev/null", lines)
    value = $$first(output)
    return($$value)
}

defineTest(qtConfReport_macosToolchainVersion) {
    arg = $$2
    contains(arg, "macosVersion"): report_message = $$webEngineGetMacOSVersion()
    contains(arg, "xcodeVersion"): report_message = "$$QMAKE_XCODE_VERSION"
    contains(arg, "clangVersion"): report_message = $$webEngineGetMacOSClangVerboseVersion()
    contains(arg, "sdkVersion"): report_message = $$webEngineGetMacOSSDKVersion()
    contains(arg, "deploymentTarget"): report_message = "$$QMAKE_MACOSX_DEPLOYMENT_TARGET"
    !isEmpty(report_message): qtConfReportPadded($$1, $$report_message)
}

defineTest(qtConfTest_isWindowsHostCompiler64) {
    win_host_arch = $$(VSCMD_ARG_HOST_ARCH)
    isEmpty(win_host_arch): return(true)
    contains(win_host_arch,"x64"): return(true)
    qtLog("Required 64-bit cross-building or native toolchain was not detected.")
    return(false)
}

# Fixme QTBUG-71772
defineTest(qtConfTest_hasThumbFlag) {
    FLAG = $$qtwebengine_extractCFlag("-mthumb")
    !isEmpty(FLAG): return(true)
    FLAG = $$qtwebengine_extractCFlag("-marm")
    !isEmpty(FLAG): return(false)

    MARCH = $$qtwebengine_extractCFlag("-march=.*")
    MARMV = $$replace(MARCH, "armv",)
    !isEmpty(MARMV) {
        MARMV = $$split(MARMV,)
        MARMV = $$member(MARMV, 0)
    }
    if (isEmpty(MARMV) | lessThan(MARMV, 7)): return(false)
    # no flag assume mthumb
    return(true)
}
