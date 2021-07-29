TARGET = qtquickcontrols2fusionstyleplugin
TARGETPATH = QtQuick/Controls.2/Fusion
IMPORT_VERSION = 2.5

QT += qml quick
QT_PRIVATE += core-private gui-private qml-private quick-private quicktemplates2-private quickcontrols2-private

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

include(fusion.pri)

OTHER_FILES += \
    qmldir \
    $$QML_FILES

SOURCES += \
    $$PWD/qtquickcontrols2fusionstyleplugin.cpp

RESOURCES += \
    $$PWD/qtquickcontrols2fusionstyle.qrc

CONFIG += no_cxx_module install_qml_files builtin_resources qtquickcompiler
load(qml_plugin)

requires(qtConfig(quickcontrols2-fusion))
