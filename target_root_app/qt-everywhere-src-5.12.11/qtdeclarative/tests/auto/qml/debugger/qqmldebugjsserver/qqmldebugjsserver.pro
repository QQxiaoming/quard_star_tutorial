QT += qml testlib
macos:CONFIG -= app_bundle
INCLUDEPATH += ../shared
SOURCES += qqmldebugjsserver.cpp
DEFINES += QT_QML_DEBUG_NO_WARNING

target.path = $$[QT_INSTALL_TESTS]/qqmldebugjsserver
INSTALLS += target

