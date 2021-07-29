QT += gui waylandcompositor

HEADERS += \
    compositor.h \
    window.h

SOURCES += main.cpp \
    compositor.cpp \
    window.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/reference-cpp
INSTALLS += target
