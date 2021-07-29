TARGET = tst_qhelpgenerator
CONFIG += testcase

HEADERS += ../../../src/assistant/qhelpgenerator/helpgenerator.h \
           ../../../src/assistant/qhelpgenerator/qhelpdatainterface_p.h \
           ../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h
SOURCES += tst_qhelpgenerator.cpp \
           ../../../src/assistant/qhelpgenerator/helpgenerator.cpp \
           ../../../src/assistant/qhelpgenerator/qhelpdatainterface.cpp \
           ../../../src/assistant/qhelpgenerator/qhelpprojectdata.cpp
QT      += help-private sql testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE
