TEMPLATE = app

QT += qml quick
SOURCES += main.cpp
RESOURCES += stocqt.qrc
OTHER_FILES += *.qml content/*.qml content/images/*.png

target.path = $$[QT_INSTALL_EXAMPLES]/demos/stocqt
INSTALLS += target
