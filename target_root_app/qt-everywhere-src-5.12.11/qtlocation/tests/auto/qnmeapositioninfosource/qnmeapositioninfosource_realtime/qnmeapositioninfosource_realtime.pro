TEMPLATE = app
CONFIG+=testcase
QT += network positioning testlib
TARGET = tst_qnmeapositioninfosource_realtime

INCLUDEPATH += ..

HEADERS += ../../utils/qlocationtestutils_p.h \
           ../../qgeopositioninfosource/testqgeopositioninfosource_p.h \
           ../qnmeapositioninfosourceproxyfactory.h \
           ../tst_qnmeapositioninfosource.h

SOURCES += ../../utils/qlocationtestutils.cpp \
           ../../qgeopositioninfosource/testqgeopositioninfosource.cpp \
           ../qnmeapositioninfosourceproxyfactory.cpp \
           ../tst_qnmeapositioninfosource.cpp \
           tst_qnmeapositioninfosource_realtime.cpp

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
