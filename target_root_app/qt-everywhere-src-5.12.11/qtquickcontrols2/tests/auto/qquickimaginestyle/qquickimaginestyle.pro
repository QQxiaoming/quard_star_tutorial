TEMPLATE = app
TARGET = tst_qquickimaginestyle
CONFIG += qmltestcase

SOURCES += \
    $$PWD/tst_qquickimaginestyle.cpp

RESOURCES += \
    $$PWD/qtquickcontrols2.conf \
    $$PWD/control-assets/button-background.9.png

OTHER_FILES += \
    $$PWD/data/*.qml

TESTDATA += \
    $$PWD/data/tst_*
