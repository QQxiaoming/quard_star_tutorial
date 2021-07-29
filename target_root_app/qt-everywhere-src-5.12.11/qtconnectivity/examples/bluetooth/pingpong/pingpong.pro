TEMPLATE = app
TARGET = pingpong

QT += quick bluetooth
android: QT += androidextras

# Input
SOURCES += main.cpp \
    pingpong.cpp

OTHER_FILES += assets/*.qml

RESOURCES += \
    resource.qrc

HEADERS += \
    pingpong.h

target.path = $$[QT_INSTALL_EXAMPLES]/bluetooth/pingpong
INSTALLS += target
