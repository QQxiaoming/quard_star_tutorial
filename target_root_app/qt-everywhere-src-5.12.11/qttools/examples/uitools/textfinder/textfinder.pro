#! [0]
QT += widgets uitools

HEADERS = textfinder.h
SOURCES = textfinder.cpp main.cpp
RESOURCES = textfinder.qrc
#! [0]

target.path = $$[QT_INSTALL_EXAMPLES]/uitools/textfinder
INSTALLS += target
