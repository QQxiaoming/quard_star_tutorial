QT += waylandclient-private gui-private
CONFIG += wayland-scanner

WAYLANDCLIENTSOURCES += ../share-buffer.xml

SOURCES += main.cpp \
    sharebufferextension.cpp

HEADERS += \
    sharebufferextension.h

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/server-buffer/cpp-client
INSTALLS += target
