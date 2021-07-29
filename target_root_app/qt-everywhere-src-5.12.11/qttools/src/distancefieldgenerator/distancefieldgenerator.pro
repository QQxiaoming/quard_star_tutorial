TARGET = qdistancefieldgenerator

QT += gui widgets gui-private core-private quick-private

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    distancefieldmodel.cpp \
    distancefieldmodelworker.cpp

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_FOREACH

FORMS += \
    mainwindow.ui

HEADERS += \
    mainwindow.h \
    distancefieldmodel.h \
    distancefieldmodelworker.h

QMAKE_DOCS = $$PWD/doc/qtdistancefieldgenerator.qdocconf

QMAKE_TARGET_DESCRIPTION = "Qt Distance Field Generator"

load(qt_app)
