QT += serialbus widgets
requires(qtConfig(combobox))

TARGET = can
TEMPLATE = app

SOURCES += \
    bitratebox.cpp \
    connectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    sendframebox.cpp

HEADERS += \
    bitratebox.h \
    connectdialog.h \
    mainwindow.h \
    sendframebox.h

FORMS   += mainwindow.ui \
    connectdialog.ui \
    sendframebox.ui

RESOURCES += can.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/can
INSTALLS += target
