#
# Automatically detects the MyScript SDK directory and sets the following variables:
#
#   MYSCRIPT_FOUND: 0/1 MyScript SDK found
#

MYSCRIPT_FOUND = 0
isEmpty(MYSCRIPT_PATH): MYSCRIPT_PATH = $$PWD
unix:linux:!android {
    equals(QT_ARCH, "arm64") {
        MYSCRIPT_ENGINE_PATH = "engine/bin/lin-arm64"
        MYSCRIPT_VOIM_PATH = "voim/bin/lin-arm64"
    } else:equals(QT_ARCH, "arm") {
        MYSCRIPT_ENGINE_PATH = "engine/bin/lin-armv7"
        MYSCRIPT_VOIM_PATH = "voim/bin/lin-armv7"
    } else:equals(QT_ARCH, "x86_64") {
        MYSCRIPT_ENGINE_PATH = "engine/bin/lin-x64"
        MYSCRIPT_VOIM_PATH = "voim/bin/lin-x64"
    } else:equals(QT_ARCH, "x86")|equals(QT_ARCH, "i386") {
        MYSCRIPT_ENGINE_PATH = "engine/bin/lin-x86"
        MYSCRIPT_VOIM_PATH = "voim/bin/lin-x86"
    } else {
        MYSCRIPT_ENGINE_PATH = "engine/bin/lin-$$QT_ARCH"
        MYSCRIPT_VOIM_PATH = "voim/bin/lin-$$QT_ARCH"
    }
    MYSCRIPT_LIB_PREFIX = "lib"
    MYSCRIPT_LIB_SUFFIX = ".so"
    MYSCRIPT_VOIM_LIB_NAME = "libvoim.so"
    MYSCRIPT_VOIM_LIB_PATH = $$MYSCRIPT_PATH/$$MYSCRIPT_VOIM_PATH/$$MYSCRIPT_VOIM_LIB_NAME
} else:win32|win64 {
    equals(QT_ARCH, "x86_64") {
        MYSCRIPT_ENGINE_PATH = "engine/bin/win-x64"
        MYSCRIPT_VOIM_PATH = "voim/bin/win-x64"
    } else {
        MYSCRIPT_ENGINE_PATH = "engine/bin/win-x86"
        MYSCRIPT_VOIM_PATH = "voim/bin/win-x86"
    }
    MYSCRIPT_LIB_PREFIX = ""
    MYSCRIPT_LIB_SUFFIX = ".dll"
    MYSCRIPT_VOIM_LIB_NAME = "voim.lib"
    MYSCRIPT_VOIM_LIB_PATH = $$MYSCRIPT_PATH/voim/api/c/lib/$$MYSCRIPT_VOIM_LIB_NAME
    !exists($$MYSCRIPT_VOIM_LIB_PATH) {
        system(lib /def:$$MYSCRIPT_PATH/voim/api/c/lib/voim.def /OUT:$$MYSCRIPT_VOIM_LIB_PATH)
    }
}
MYSCRIPT_VOIM_LIB += \
    $$MYSCRIPT_VOIM_LIB_PATH
MYSCRIPT_ENGINE_BINS += \
    $$MYSCRIPT_PATH/$$MYSCRIPT_VOIM_PATH/$${MYSCRIPT_LIB_PREFIX}voim$${MYSCRIPT_LIB_SUFFIX} \
    $$MYSCRIPT_PATH/$$MYSCRIPT_ENGINE_PATH/$${MYSCRIPT_LIB_PREFIX}MyScriptInk$${MYSCRIPT_LIB_SUFFIX} \
    $$MYSCRIPT_PATH/$$MYSCRIPT_ENGINE_PATH/$${MYSCRIPT_LIB_PREFIX}MyScript2D$${MYSCRIPT_LIB_SUFFIX} \
    $$MYSCRIPT_PATH/$$MYSCRIPT_ENGINE_PATH/$${MYSCRIPT_LIB_PREFIX}MyScriptPrediction$${MYSCRIPT_LIB_SUFFIX} \
    $$MYSCRIPT_PATH/$$MYSCRIPT_ENGINE_PATH/$${MYSCRIPT_LIB_PREFIX}MyScriptText$${MYSCRIPT_LIB_SUFFIX} \
    $$MYSCRIPT_PATH/$$MYSCRIPT_ENGINE_PATH/$${MYSCRIPT_LIB_PREFIX}MyScriptEngine$${MYSCRIPT_LIB_SUFFIX}
MYSCRIPT_VOIM_CONF = voim/conf
MYSCRIPT_LANGUAGE_CONF = conf
MYSCRIPT_RESOURCES = resources

exists($$MYSCRIPT_VOIM_LIB_PATH): MYSCRIPT_FOUND = 1
