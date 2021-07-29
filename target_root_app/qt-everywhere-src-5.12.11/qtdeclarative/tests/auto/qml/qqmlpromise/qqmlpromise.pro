CONFIG += testcase
TARGET = tst_qqmlpromise
macos:CONFIG -= app_bundle

SOURCES += tst_qqmlpromise.cpp

OTHER_FILES +=

include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private qml-private testlib
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

DISTFILES += \
    data/then-fulfilled-non-callable.qml \
    data/then-reject-non-callable.qml \
    data/then-resolve-multiple-then.qml \
    data/then-resolve-chaining.qml \
    data/then-reject-chaining.qml \
    data/promise-resolve-with-value.qml \
    data/promise-resolve-with-promise.qml \
    data/promise-reject-with-value.qml \
    data/promise-executor-resolve.qml \
    data/promise-get-length.qml \
    data/promise-executor-reject.qml \
    data/promise-reject-catch.qml \
    data/promise-async-resolve-with-value.qml \
    data/promise-async-reject-with-value.qml \
    data/promise-all-resolve.qml \
    data/promise-all-empty-input.qml \
    data/promise-resolve-with-array.qml \
    data/promise-all-reject-reject-is-last.qml \
    data/promise-all-reject-reject-is-mid.qml \
    data/promise-race-resolve-1st.qml \
    data/promise-race-empty-input.qml \
    data/promise-race-resolve-2nd.qml \
    data/promise-race-resolve-1st-in-executor-function.qml \
    data/promise-resolve-is-a-function.qml \
    data/promise-resolve-function-length.qml \
    data/promise-all-invoke-then-method.qml \
    data/promise-resolve-with-empty.qml \
    data/promise-executor-throw-exception.qml \
    data/promise-executor-function-extensible.qml \
    data/promise-all-noniterable-input.qml
