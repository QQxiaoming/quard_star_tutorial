TEMPLATE     = app

QT          += qml quick

SOURCES     += main.cpp
RESOURCES   += clocks.qrc

target.path  = $$[QT_INSTALL_EXAMPLES]/demos/clocks
INSTALLS    += target

OTHER_FILES  += \
                clocks.qml \
                content/Clock.qml \
                content/*.png
