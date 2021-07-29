!include( ../examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}

SOURCE += main.cpp

QT += qml quick 3dcore 3drender 3dinput 3dquick 3dquickextras

OTHER_FILES += \
    main.qml \
    CubeEntity.qml

SOURCES += \
    main.cpp

RESOURCES += \
    phong-cubes.qrc \
    ../exampleresources/textures.qrc
