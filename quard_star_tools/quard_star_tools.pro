!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets xml svg websockets multimedia

BUILD_VERSION=0.2.0
TARGET_ARCH=$${QT_ARCH}
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION="\\\"V$${BUILD_VERSION}\\\""
QMAKE_CXXFLAGS += -Wno-deprecated-copy

include(./lib/QFontIcon/QFontIcon.pri)
include(./lib/lwext4/lwext4.pri)
include(./lib/ff15/ff15.pri)
include(./lib/jffs2/jffs2.pri)
include(./lib/qxymodem/qxymodem.pri)
include(./lib/qzmodem/qzmodem.pri)
include(./lib/qtermwidget/qtermwidget.pri)
include(./lib/QSourceHighlite/QSourceHighlite.pri)
include(./lib/QVNCClient/QVNCClient.pri)
include(./lib/QTelnet/QTelnet.pri)

INCLUDEPATH += \
        -I . \
        -I ./src \
        -I ./src/boardview \
        -I ./src/telnetview \
        -I ./src/vncview

SOURCES += \
    src/main.cpp \
    src/boardview/boardwindow.cpp \
    src/boardview/netselectbox.cpp \
    src/boardview/bootselectbox.cpp \
    src/boardview/treemodel.cpp \
    src/boardview/fsviewmodel.cpp \
    src/boardview/qfsviewer.cpp \
    src/telnetview/telnetwindow.cpp \
    src/telnetview/asciibox.cpp \
    src/vncview/vncwindow.cpp

HEADERS += \
    src/boardview/boardwindow.h \
    src/boardview/netselectbox.h \
    src/boardview/bootselectbox.h \
    src/boardview/treemodel.h \
    src/boardview/fsviewmodel.h \
    src/boardview/qfsviewer.h \
    src/telnetview/telnetwindow.h \
    src/telnetview/asciibox.h \
    src/vncview/vncwindow.h

FORMS += \
    src/boardview/boardwindow.ui \
    src/boardview/netselectbox.ui \
    src/boardview/bootselectbox.ui \
    src/telnetview/telnetwindow.ui \
    src/telnetview/asciibox.ui \
    src/vncview/vncwindow.ui

RESOURCES += \
    res/resource.qrc \
    res/terminalfont.qrc \
    theme/dark/darkstyle.qrc \
    theme/light/lightstyle.qrc

TRANSLATIONS += \
    lang/quard_star_tools_zh_CN.ts \
    lang/quard_star_tools_ja_JP.ts \
    lang/quard_star_tools_en_US.ts

build_type =
CONFIG(debug, debug|release) {
    build_type = debug
} else {
    build_type = release
}

DESTDIR     = $$build_type/out
OBJECTS_DIR = $$build_type/obj
MOC_DIR     = $$build_type/moc
RCC_DIR     = $$build_type/rcc
UI_DIR      = $$build_type/ui

win32:!wasm {
    DEFINES += DESKTOP_INTERACTION_MODE
    DEFINES += BUILT_IN_QEMU_MODE
    
    VERSION = $${BUILD_VERSION}.000
    RC_LANG = 0x0004
    RC_ICONS = "icons\icon.ico"

    contains(TARGET_ARCH, x86_64) {
        CONFIG(release, debug|release) {
            AFTER_LINK_CMD_LINE = $$PWD/tools/upx-3.96-win64/upx.exe --best -f $$DESTDIR/$${TARGET}.exe
            QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
        }
    } else {
        QMAKE_LFLAGS += -Wl,--large-address-aware
        CONFIG(release, debug|release) {
            AFTER_LINK_CMD_LINE = $$PWD/tools/upx-3.96-win32/upx.exe --best -f $$DESTDIR/$${TARGET}.exe
            QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
        }
    }
}

unix:!macx:!android:!ios:!wasm {
    DEFINES += DESKTOP_INTERACTION_MODE
    DEFINES += BUILT_IN_QEMU_MODE

    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie

    CONFIG(release, debug|release) {
        AFTER_LINK_CMD_LINE = upx-ucl --best -f $$DESTDIR/$$TARGET
        QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
    }

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

macx:!ios:!wasm {
    DEFINES += DESKTOP_INTERACTION_MODE
    DEFINES += BUILT_IN_QEMU_MODE

    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons/icon.icns"

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

android { 
    DEFINES += MOBILE_INTERACTION_MODE
    DEFINES += EXTERNAL_QEMU_MODE

    DISTFILES += \
        platform/android/AndroidManifest.xml \
        platform/android/res/values/libs.xml \
        platform/android/gradle.properties \
        platform/android/gradle/wrapper/gradle-wrapper.jar \
        platform/android/gradle/wrapper/gradle-wrapper.properties \
        platform/android/gradlew \
        platform/android/gradlew.bat
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/platform/android

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

ios {
    DEFINES += MOBILE_INTERACTION_MODE
    DEFINES += EXTERNAL_QEMU_MODE

    CONFIG += hide_symbols
    QMAKE_INFO_PLIST = platform/ios/Info.plist
    ios_icon.files = $$files($$PWD/platform/ios/AppIcon*.png)
    ios_launch.files = $$files($$PWD/platform/ios/Soccer.png)
    ios_settings.files = $$files($$PWD/platform/ios/Settings.bundle)
    QMAKE_BUNDLE_DATA += ios_icon
    QMAKE_BUNDLE_DATA += ios_launch
    QMAKE_BUNDLE_DATA += ios_settings
    QMAKE_IOS_LAUNCH_SCREEN = $$PWD/platform/ios/Launch.storyboard
    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

wasm {
    DEFINES += DESKTOP_INTERACTION_MODE
    DEFINES += EXTERNAL_QEMU_MODE
    
    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")

    AFTER_LINK_CMD_LINE = cp $$PWD/platform/webassembly/$${TARGET}.html $$DESTDIR/ ; cp $$PWD/icons/board.gif $$DESTDIR/ ; cp $$PWD/icons/icon.ico $$DESTDIR/favicon.ico ; rm -rf $$DESTDIR/qtlogo.svg
    QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
}

git_tag.target = $$PWD/git_tag.inc
git_tag.depends = FORCE
PRE_TARGETDEPS += $$PWD/git_tag.inc
QMAKE_EXTRA_TARGETS += git_tag
