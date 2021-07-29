TARGET = Qt3DCoreTest
MODULE = 3dcoretest
CONFIG += static internal_module

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_FOREACH
PRECOMPILED_HEADER =
INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/testpostmanarbiter.cpp

HEADERS += \
    $$PWD/testpostmanarbiter_p.h

qtConfig(private_tests) {
    SOURCES += \
        $$PWD/qbackendnodetester.cpp

    HEADERS += \
        $$PWD/qbackendnodetester_p.h
}

QT += core-private 3dcore 3dcore-private

load(qt_module)
