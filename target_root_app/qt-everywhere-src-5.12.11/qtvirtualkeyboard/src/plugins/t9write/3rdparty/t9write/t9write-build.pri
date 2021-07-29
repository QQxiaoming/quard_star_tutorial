#
# Automatically detects the T9Write build directory and sets the following variables:
#
#   T9WRITE_FOUND: 0/1 T9Write SDK found
#   T9WRITE_BUILD_STATIC: 0/1 Static libraries found (0 == shared libraries)
#   T9WRITE_ALPHABETIC_FOUND: 0/1 T9 Write Alphabetic API header found
#   T9WRITE_CJK_FOUND: 0/1 T9 Write CJK API header found
#   T9WRITE_INCLUDE_DIRS: T9 Write include directories
#   T9WRITE_ALPHABETIC_LIBS: Absolute path to the target library file
#   T9WRITE_ALPHABETIC_BINS: Absolute path to the target binary file (shared library)
#   T9WRITE_CJK_LIBS: Absolute path to the target library file
#   T9WRITE_CJK_BINS: Absolute path to the target binary file (shared library)
#

T9WRITE_FOUND = 0
T9WRITE_ALPHABETIC_FOUND = 0
T9WRITE_CJK_FOUND = 0
T9WRITE_INCLUDE_DIRS = $$PWD/api
contains(QT_ARCH, arm) {
    T9WRITE_BUILD_SHARED_DIR = lib/arm/shared
    T9WRITE_BUILD_STATIC_DIR = lib/arm/static
} else:linux {
    T9WRITE_BUILD_SHARED_DIR = lib/linux/shared
    T9WRITE_BUILD_STATIC_DIR = lib/linux/static
} else:win32 {
    T9WRITE_BUILD_SHARED_DIR = lib/win32/shared
    T9WRITE_BUILD_STATIC_DIR = lib/win32/static
}

defineReplace(findStaticLibrary) {
    win32 {
        result = $$files($$1/*.obj)
        isEmpty(result): result = $$files($$1/*.lib)
    } else {
        result = $$files($$1/*.o)
        isEmpty(result): result = $$files($$1/*.a)
    }
    return($$result)
}

defineReplace(findSharedLibrary) {
    win32 {
        result = $$files($$1/*.lib)
    } else {
        result = $$files($$1/*.so)
    }
    return($$result)
}

defineReplace(findSharedBinary) {
    win32 {
        result = $$files($$1/*.dll)
    } else {
        result = $$files($$1/*.so)
    }
    return($$result)
}

for(include_dir, T9WRITE_INCLUDE_DIRS) {
    exists($${include_dir}/decuma_hwr.h): T9WRITE_ALPHABETIC_FOUND = 1
    exists($${include_dir}/decuma_hwr_cjk.h): T9WRITE_CJK_FOUND = 1
}

equals(T9WRITE_ALPHABETIC_FOUND, 1)|equals(T9WRITE_CJK_FOUND, 1) {
    equals(T9WRITE_ALPHABETIC_FOUND, 1) {
        T9WRITE_ALPHABETIC_LIBS = $$findSharedLibrary($$PWD/$$T9WRITE_BUILD_SHARED_DIR/alphabetic)
        !isEmpty(T9WRITE_ALPHABETIC_LIBS) {
            T9WRITE_ALPHABETIC_BINS = $$findSharedBinary($$PWD/$$T9WRITE_BUILD_SHARED_DIR/alphabetic)
        } else {
            T9WRITE_ALPHABETIC_LIBS = $$findStaticLibrary($$PWD/$$T9WRITE_BUILD_STATIC_DIR/alphabetic)
        }
    }
    equals(T9WRITE_CJK_FOUND, 1) {
        T9WRITE_CJK_LIBS = $$findSharedLibrary($$PWD/$$T9WRITE_BUILD_SHARED_DIR/cjk)
        !isEmpty(T9WRITE_CJK_LIBS) {
            T9WRITE_CJK_BINS = $$findSharedBinary($$PWD/$$T9WRITE_BUILD_SHARED_DIR/cjk)
        } else {
            T9WRITE_CJK_LIBS = $$findStaticLibrary($$PWD/$$T9WRITE_BUILD_STATIC_DIR/cjk)
        }
    }
    equals(T9WRITE_ALPHABETIC_FOUND, 1):!isEmpty(T9WRITE_ALPHABETIC_LIBS): T9WRITE_FOUND = 1
    equals(T9WRITE_CJK_FOUND, 1):!isEmpty(T9WRITE_CJK_LIBS): T9WRITE_FOUND = 1
}
