QT += gui

INCLUDEPATH += \
        -I $$PWD 

HEADERS += $$PWD/qsourcehighliter.h \
           $$PWD/qsourcehighliterthemes.h \
           $$PWD/languagedata.h

SOURCES += $$PWD/qsourcehighliter.cpp \
    $$PWD/languagedata.cpp \
    $$PWD/qsourcehighliterthemes.cpp
