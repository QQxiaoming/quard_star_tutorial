TEMPLATE = app

TARGET = tst_objgeometryloader

QT += 3dcore 3dcore-private 3drender 3drender-private testlib

CONFIG += testcase

SOURCES += \
    tst_objgeometryloader.cpp

OTHER_FILES += \
    invalid_vertex_position.obj

RESOURCES += \
    resources.qrc
