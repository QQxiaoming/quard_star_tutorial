BUILD_VERSION=0.2.0
QT += core gui network
QT += xml svg
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}

CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION="\\\"V$${BUILD_VERSION}\\\""
QMAKE_CXXFLAGS += -Wno-deprecated-copy

include(./lib/QFontIcon/QFontIcon.pri)
include(./lib/lwext4/lwext4.pri)
include(./lib/ff15/ff15.pri)
include(./lib/jffs2/jffs2.pri)
include(./lib/qtxyzmodem/qtxyzmodem.pro)
include(./lib/qtermwidget/qtermwidget.pro)

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
    src/telnetview/QTelnet.cpp \
    src/telnetview/telnetwindow.cpp \
    src/telnetview/asciibox.cpp \
    src/vncview/qvncclientwidget.cpp \
    src/vncview/vncwindow.cpp

HEADERS += \
    src/boardview/boardwindow.h \
    src/boardview/netselectbox.h \
    src/boardview/bootselectbox.h \
    src/boardview/treemodel.h \
    src/boardview/fsviewmodel.h \
    src/boardview/qfsviewer.h \
    src/telnetview/QTelnet.h \
    src/telnetview/telnetwindow.h \
    src/telnetview/asciibox.h \
    src/vncview/qvncclientwidget.h \
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

win32:{
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

unix:!macx:!android:!ios {
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie

    CONFIG(release, debug|release) {
        AFTER_LINK_CMD_LINE = upx-ucl --best -f $$DESTDIR/$$TARGET
        QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
    }

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

macx:!ios{
    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons/icon.icns"

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

android { 
    DEFINES += MOBILE_MODE
    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

ios {
    DEFINES += MOBILE_MODE
    CONFIG += hide_symbols
    QMAKE_TARGET_BUNDLE_PREFIX = com.quard
    QMAKE_BUNDLE = $${TARGET}
    QMAKE_INFO_PLIST = platform/ios/Info.plist
    ios_icon.files = $$files($$PWD/platform/ios/AppIcon*.png)
    ios_launch.files = $$files($$PWD/platform/ios/Soccer.png)
    QMAKE_BUNDLE_DATA += ios_icon
    QMAKE_BUNDLE_DATA += ios_launch
    QMAKE_IOS_LAUNCH_SCREEN = $$PWD/platform/ios/Launch.storyboard
    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

git_tag.target = $$PWD/git_tag.inc
git_tag.depends = FORCE
PRE_TARGETDEPS += $$PWD/git_tag.inc
QMAKE_EXTRA_TARGETS += git_tag
