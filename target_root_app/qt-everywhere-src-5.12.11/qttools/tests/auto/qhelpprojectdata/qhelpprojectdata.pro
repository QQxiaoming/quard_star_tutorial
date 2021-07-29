TARGET = tst_qhelpprojectdata
CONFIG += testcase

HEADERS += ../../../src/assistant/qhelpgenerator/qhelpdatainterface_p.h \
           ../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h

SOURCES += tst_qhelpprojectdata.cpp \
           ../../../src/assistant/qhelpgenerator/qhelpdatainterface.cpp \
           ../../../src/assistant/qhelpgenerator/qhelpprojectdata.cpp

QT      += help-private testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE

