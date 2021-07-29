QT = network core qmldebug-private
CONFIG += no_import_scan

SOURCES += \
    main.cpp \
    qmlpreviewapplication.cpp \
    qmlpreviewfilesystemwatcher.cpp

HEADERS += \
    qmlpreviewapplication.h \
    qmlpreviewfilesystemwatcher.h

QMAKE_TARGET_DESCRIPTION = QML Preview

load(qt_tool)
