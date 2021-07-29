QT += network help-private

QTPLUGIN.platforms = qminimal

SOURCES += ../shared/collectionconfiguration.cpp \
           helpgenerator.cpp \
           collectionconfigreader.cpp \
           qhelpprojectdata.cpp \
           qhelpdatainterface.cpp \
           main.cpp

HEADERS += ../shared/collectionconfiguration.h \
           helpgenerator.h \
           collectionconfigreader.h \
           qhelpprojectdata_p.h \
           qhelpdatainterface_p.h

QMAKE_TARGET_DESCRIPTION = "Qt Compressed Help File Generator"
load(qt_tool)
