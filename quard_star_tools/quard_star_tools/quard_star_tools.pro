QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += APP_VERSION="\\\"V0.0.1\\\""

INCLUDEPATH += \
        -I . \
        -I ./boardview \
        -I ./telnetview \
        -I ./vncview

SOURCES += \
    main.cpp \
    boardview/boardwindow.cpp \
    telnetview/QTelnet.cpp \
    telnetview/telnetwindow.cpp \
    vncview/qvncclientwidget.cpp \
    vncview/vncwindow.cpp

HEADERS += \
    boardview/boardwindow.h \
    telnetview/QTelnet.h \
    telnetview/telnetwindow.h \
    vncview/qvncclientwidget.h \
    vncview/vncwindow.h

FORMS += \
    boardview/boardwindow.ui \
    telnetview/telnetwindow.ui \
    vncview/vncwindow.ui

RESOURCES += \
    res/images.qrc

TRANSLATIONS += \
    lang/quard_star_tools_zh_CN.ts

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
    RC_LANG = 0x0004
    VERSION = 0.0.1.0
}

unix:!macx:{
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

macx:{
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie

    git_tag.commands = $$quote("cd $$PWD && git describe --always --long --dirty --abbrev=10 --exclude '*' | awk \'{print \"\\\"\"\$$0\"\\\"\"}\' > git_tag.tmp && mv git_tag.tmp git_tag.inc")
}

git_tag.target = $$PWD/git_tag.inc
git_tag.depends = FORCE
PRE_TARGETDEPS += $$PWD/git_tag.inc
QMAKE_EXTRA_TARGETS += git_tag
