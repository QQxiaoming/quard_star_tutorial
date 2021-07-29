QT      +=  webenginewidgets

HEADERS =   mainwindow.h
SOURCES =   main.cpp \
            mainwindow.cpp
RESOURCES = jquery.qrc

# Disable Qt Quick compiler because the example doesn't use QML, but more importantly so that
# the source code of the .js files is not removed from the embedded qrc file.
CONFIG -= qtquickcompiler

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/contentmanipulation
INSTALLS += target
