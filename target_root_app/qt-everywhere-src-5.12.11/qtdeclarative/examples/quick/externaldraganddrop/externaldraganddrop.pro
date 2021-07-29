TEMPLATE = app

QT += quick qml
SOURCES += main.cpp
RESOURCES += externaldraganddrop.qrc ../shared/shared.qrc

EXAMPLE_FILES = \
    externaldraganddrop.qml \
    DragAndDropTextItem.qml

target.path = $$[QT_INSTALL_EXAMPLES]/quick/externaldraganddrop
INSTALLS += target
