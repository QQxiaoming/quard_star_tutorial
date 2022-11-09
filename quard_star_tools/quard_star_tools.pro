BUILD_VERSION=0.1.1
QT += core gui network
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

INCLUDEPATH += \
        -I . \
        -I ./boardview \
        -I ./telnetview \
        -I ./vncview

SOURCES += \
    main.cpp \
    boardview/boardwindow.cpp \
    boardview/netselectbox.cpp \
    telnetview/QTelnet.cpp \
    telnetview/telnetwindow.cpp \
    vncview/qvncclientwidget.cpp \
    vncview/vncwindow.cpp

HEADERS += \
    boardview/boardwindow.h \
    boardview/netselectbox.h \
    telnetview/QTelnet.h \
    telnetview/telnetwindow.h \
    vncview/qvncclientwidget.h \
    vncview/vncwindow.h

FORMS += \
    boardview/boardwindow.ui \
    boardview/netselectbox.ui \
    telnetview/telnetwindow.ui \
    vncview/vncwindow.ui

RESOURCES += \
    res/resource.qrc

TRANSLATIONS += \
    lang/quard_star_tools_zh_CN.ts \
    lang/quard_star_tools_ja_JP.ts \
    lang/quard_star_tools_en_US.ts

# 输出配置
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

# 平台配置
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

unix:!macx:{
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie

    CONFIG(release, debug|release) {
        AFTER_LINK_CMD_LINE = upx-ucl --best -f $$DESTDIR/$$TARGET
        QMAKE_POST_LINK += $$quote($$AFTER_LINK_CMD_LINE)
    }

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

macx:{
    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons/icon.icns"

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

git_tag.target = $$PWD/git_tag.inc
git_tag.depends = FORCE
PRE_TARGETDEPS += $$PWD/git_tag.inc
QMAKE_EXTRA_TARGETS += git_tag
