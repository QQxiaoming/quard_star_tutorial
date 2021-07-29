QT += gui waylandcompositor

LIBS += -L ../../lib

HEADERS += \
    window.h \
    compositor.h

SOURCES += main.cpp \
    window.cpp \
    compositor.cpp

# to make QtWaylandCompositor/... style includes working without installing
INCLUDEPATH += $$PWD/../../include


RESOURCES += qwindow-compositor.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/qwindow-compositor
INSTALLS += target
