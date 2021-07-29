TEMPLATE = app
TARGET = sidepanel
QT += quick

SOURCES += \
    sidepanel.cpp

RESOURCES += \
    doc/images/qtquickcontrols2-sidepanel-landscape.png \
    doc/images/qtquickcontrols2-sidepanel-portrait.png \
    images/qt-logo@2x.png \
    images/qt-logo@3x.png \
    images/qt-logo@4x.png \
    images/qt-logo.png \
    sidepanel.qml

target.path = $$[QT_INSTALL_EXAMPLES]/quickcontrols2/sidepanel
INSTALLS += target
