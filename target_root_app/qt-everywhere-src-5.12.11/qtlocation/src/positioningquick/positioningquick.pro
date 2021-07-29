TARGET = QtPositioningQuick
QT = quick-private positioning-private qml-private core-private
CONFIG += simd optimize_full

INCLUDEPATH += $$PWD

SOURCES += $$files(*.cpp)
HEADERS += $$files(*.h)

load(qt_module)
