TEMPLATE = app
CONFIG += testcase
TARGET = tst_oauth1
SOURCES  += tst_oauth1.cpp

include(../shared/shared.pri)

QT = core core-private network networkauth networkauth-private testlib
