QT += gui qml

SOURCES += \
    main.cpp

OTHER_FILES = \
    main.qml

RESOURCES += hwlayer-compositor.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/hwlayer-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS hwlayer-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/wayland/hwlayer-compositor
INSTALLS += target sources
