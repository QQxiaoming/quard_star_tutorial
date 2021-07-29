!include( ../manual.pri ) {
    error( "Couldn't find the manual.pri file!" )
}

QT += 3dquickextras 3dcore 3drender 3dinput 3dquick qml quick 3dquickrender

SOURCES += main.cpp

RESOURCES += \
    video-texture-qml.qrc

OTHER_FILES += \
    main.qml

DISTFILES +=

