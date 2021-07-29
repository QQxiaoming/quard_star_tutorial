TEMPLATE = app

QT += quick qml
SOURCES += \
        main.cpp \
    gameoflifemodel.cpp

RESOURCES += \
    main.qml \
    gosperglidergun.cells

target.path = $$[QT_INSTALL_EXAMPLES]/quick/tableview/gameoflife
INSTALLS += target

HEADERS += \
    gameoflifemodel.h
