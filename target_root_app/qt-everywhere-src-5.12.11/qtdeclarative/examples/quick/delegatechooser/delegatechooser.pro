TEMPLATE = app
QT += quick qml

SOURCES += main.cpp
RESOURCES += qml.qrc ../shared/shared.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/quick/delegatechooser
INSTALLS += target
