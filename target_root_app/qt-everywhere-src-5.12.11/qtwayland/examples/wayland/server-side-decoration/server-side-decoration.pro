QT += gui qml

SOURCES += \
    main.cpp

OTHER_FILES = \
    main.qml

RESOURCES += server-side-decoration.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/wayland/server-side-decoration
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS server-side-decoration.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/wayland/server-side-decoration
INSTALLS += target sources
