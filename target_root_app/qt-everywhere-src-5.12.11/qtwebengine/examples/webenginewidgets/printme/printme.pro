QT += webenginewidgets printsupport

HEADERS   = printhandler.h
SOURCES   = main.cpp \
            printhandler.cpp
RESOURCES = data/data.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/printme
INSTALLS += target
