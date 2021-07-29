TEMPLATE = app
TARGET = tst_qquickcolor
CONFIG += qmltestcase

SOURCES += \
    $$PWD/tst_qquickcolor.cpp

OTHER_FILES += \
    $$PWD/data/*.qml

TESTDATA += \
    $$PWD/data/tst_*
