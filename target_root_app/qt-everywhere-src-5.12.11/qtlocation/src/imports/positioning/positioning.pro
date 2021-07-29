QT += quick-private positioning-private positioningquick-private qml-private core-private

INCLUDEPATH *= $$PWD

HEADERS += $$files(*.h)
SOURCES += $$files(*.cpp)

load(qml_plugin)

OTHER_FILES += \
    plugin.json \
    qmldir
