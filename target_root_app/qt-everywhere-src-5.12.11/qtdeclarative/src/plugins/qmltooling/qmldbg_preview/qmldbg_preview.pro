QT += core-private qml-private packetprotocol-private network quick-private gui-private

TARGET = qmldbg_preview

SOURCES += \
    $$PWD/qqmlpreviewblacklist.cpp \
    $$PWD/qqmlpreviewfileengine.cpp \
    $$PWD/qqmlpreviewfileloader.cpp \
    $$PWD/qqmlpreviewhandler.cpp \
    $$PWD/qqmlpreviewposition.cpp \
    $$PWD/qqmlpreviewservice.cpp \
    $$PWD/qqmlpreviewservicefactory.cpp

HEADERS += \
    $$PWD/qqmlpreviewblacklist.h \
    $$PWD/qqmlpreviewfileengine.h \
    $$PWD/qqmlpreviewfileloader.h \
    $$PWD/qqmlpreviewhandler.h \
    $$PWD/qqmlpreviewposition.h \
    $$PWD/qqmlpreviewservice.h \
    $$PWD/qqmlpreviewservicefactory.h

OTHER_FILES += \
    $$PWD/qqmlpreviewservice.json

PLUGIN_TYPE = qmltooling
PLUGIN_CLASS_NAME = QQmlPreviewServiceFactory

load(qt_plugin)
