!include( ../manual.pri ) {
    error( "Couldn't find the manual.pri file!" )
}

QT += 3dextras 3dcore 3drender 3dinput 3dquick qml quick

SOURCES += main.cpp

RESOURCES += \
    render-qml-to-texture-qml.qrc

OTHER_FILES += \
    main.qml

HEADERS +=


