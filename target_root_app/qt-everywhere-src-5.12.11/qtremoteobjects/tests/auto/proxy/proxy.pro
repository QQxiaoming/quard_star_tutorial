CONFIG += testcase
TARGET = tst_proxy
QT += testlib remoteobjects
QT -= gui

SOURCES += tst_proxy.cpp
HEADERS += $$PWD/../shared/model_utilities.h

REPC_MERGED += engine.rep
REPC_MERGED += subclass.rep

contains(QT_CONFIG, c++11): CONFIG += c++11
