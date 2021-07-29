QT += gui qml

SOURCES += \
    main.cpp

OTHER_FILES = \
    main.qml

RESOURCES += overview-compositor.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/overview-compositor
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS overview-compositor.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/wayland/overview-compositor
INSTALLS += target sources
