TARGET = tst_integration

OTHER_FILES = engine.rep \
              ../repfiles/localdatacenter.rep \
              ../repfiles/tcpdatacenter.rep

REPC_SOURCE += $$OTHER_FILES
REPC_REPLICA += $$OTHER_FILES
REPC_MERGED += speedometer.rep enum.rep pod.rep

HEADERS += engine.h \
           speedometer.h \
           temperature.h

SOURCES += engine.cpp \
           speedometer.cpp \
           tst_integration.cpp

CONFIG += testcase
QT += testlib remoteobjects
QT -= gui

contains(QT_CONFIG, c++11): CONFIG += c++11
