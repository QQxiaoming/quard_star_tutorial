greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport
    requires(qtConfig(combobox))
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

TARGET = slave
TEMPLATE = app

HEADERS += \
    dialog.h

SOURCES += \
    main.cpp \
    dialog.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/slave
INSTALLS += target
