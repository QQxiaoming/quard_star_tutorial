TEMPLATE = app

QT += quick qml
SOURCES += main.cpp
RESOURCES += \
    ../images.qrc \
    ../../shared/shared.qrc \
    $$files("content/*") \
    $$files("*qml")

target.path = $$[QT_INSTALL_EXAMPLES]/quick/particles/itemparticle
INSTALLS += target
