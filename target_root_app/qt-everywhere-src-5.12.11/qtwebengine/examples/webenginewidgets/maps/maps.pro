TEMPLATE = app

QT += webenginewidgets

HEADERS += \
    mainwindow.h

SOURCES += main.cpp \
    mainwindow.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/maps
INSTALLS += target

!qtConfig(webengine-geolocation) {
    error('Qt WebEngine compiled without geolocation support, this example will not work.')
}

