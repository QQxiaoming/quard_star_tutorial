
INCLUDEPATH += $$PWD
HEADERS += $$PWD/util.h
SOURCES += $$PWD/util.cpp

android|ios {
    DEFINES += QT_QMLTEST_DATADIR=\\\":/data\\\"
} else {
    DEFINES += QT_QMLTEST_DATADIR=\\\"$${_PRO_FILE_PWD_}/data\\\"
}
