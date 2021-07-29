CONFIG += testcase
TARGET = tst_ecmascripttests
QT += testlib qml-private
macos:CONFIG -= app_bundle
SOURCES += tst_ecmascripttests.cpp qjstest/test262runner.cpp
HEADERS += qjstest/test262runner.h
DEFINES += SRCDIR=\\\"$$PWD\\\"

# The ES test suite takes approximately 5 mins to run, on a fairly
# vanilla developer machine, so the default watchdog timer kills the
# test some of the time.  Fix by raising time-out to 400s when
# invoking tst_ecmascripttests:
checkenv.name = QTEST_FUNCTION_TIMEOUT
checkenv.value = 500000
QT_TOOL_ENV += checkenv
