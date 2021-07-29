QT += core gui qml

QT += waylandcompositor-private

CONFIG += wayland-scanner
CONFIG += c++11
SOURCES += \
    main.cpp \
    sharebufferextension.cpp

OTHER_FILES = \
    qml/main.qml \
    qml/Screen.qml \
    images/background.jpg

WAYLANDSERVERSOURCES += \
    ../share-buffer.xml

RESOURCES += compositor.qrc

TARGET = compositor

HEADERS += \
    sharebufferextension.h

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/server-buffer/compositor
INSTALLS += target
