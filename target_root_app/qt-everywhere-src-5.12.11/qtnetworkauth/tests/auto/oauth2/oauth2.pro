TEMPLATE = app
CONFIG += testcase
TARGET = tst_oauth2
SOURCES  += tst_oauth2.cpp

include(../shared/shared.pri)

QT = core core-private network networkauth networkauth-private testlib
