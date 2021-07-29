TEMPLATE = app
TARGET = automotive
QT += quick quickcontrols2

SOURCES += \
    automotive.cpp

RESOURCES += \
    icons/icons.qrc \
    imagine-assets/imagine-assets.qrc \
    qml/qml.qrc \
    qtquickcontrols2.conf

target.path = $$[QT_INSTALL_EXAMPLES]/quickcontrols2/imagine/automotive
INSTALLS += target
