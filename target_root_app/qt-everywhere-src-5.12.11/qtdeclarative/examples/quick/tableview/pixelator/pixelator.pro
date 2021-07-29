TEMPLATE = app

QT += quick qml
HEADERS += imagemodel.h
SOURCES += main.cpp \
        imagemodel.cpp

RESOURCES += qt.png main.qml

target.path = $$[QT_INSTALL_EXAMPLES]/quick/tableview/pixelator
INSTALLS += target
