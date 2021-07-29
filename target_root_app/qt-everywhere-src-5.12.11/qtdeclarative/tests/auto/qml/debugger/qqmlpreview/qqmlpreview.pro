CONFIG += testcase
TARGET = tst_qqmlpreview

QT += qml testlib core qmldebug-private
macos:CONFIG -= app_bundle

INCLUDEPATH += ../../../../../src/plugins/qmltooling/qmldbg_preview/

SOURCES += \
    tst_qqmlpreview.cpp \
    ../../../../../src/plugins/qmltooling/qmldbg_preview/qqmlpreviewblacklist.cpp

HEADERS += \
    ../../../../../src/plugins/qmltooling/qmldbg_preview/qqmlpreviewblacklist.h

include(../shared/debugutil.pri)

TESTDATA = \
    data/window.qml \
    data/qtquick2.qml \
    data/window2.qml \
    data/window1.qml \
    data/broken.qml \
    data/zoom.qml \
    data/i18n/qml_fr_FR.qm
