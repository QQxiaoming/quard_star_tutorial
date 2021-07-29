#! [0]
QT += widgets

HEADERS = calculatorform.h
SOURCES = calculatorform.cpp main.cpp
FORMS = calculatorform.ui
#! [0]

target.path = $$[QT_INSTALL_EXAMPLES]/uitools/multipleinheritance
INSTALLS += target

