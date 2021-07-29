CONFIG += testcase
TARGET = tst_qqmldebugjs
QT += qml testlib gui-private core-private
macos:CONFIG -= app_bundle

SOURCES += tst_qqmldebugjs.cpp

INCLUDEPATH += ../shared
include(../shared/debugutil.pri)
include(../shared/qqmlenginedebugclient.pri)

TESTDATA = data/*

OTHER_FILES += data/test.qml data/test.js \
    data/timer.qml \
    data/exception.qml \
    data/oncompleted.qml \
    data/loadjsfile.qml \
    data/condition.qml \
    data/changeBreakpoint.qml \
    data/stepAction.qml \
    data/breakpointRelocation.qml \
    data/createComponent.qml \
    data/encodeQmlScope.qml \
    data/breakOnAnchor.qml
