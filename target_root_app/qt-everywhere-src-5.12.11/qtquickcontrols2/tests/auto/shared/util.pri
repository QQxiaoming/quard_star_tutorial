QT += testlib-private core-private gui-private qml-private quick-private quicktemplates2-private quickcontrols2

HEADERS += $$PWD/visualtestutil.h \
           $$PWD/util.h \
           $$PWD/qtest_quickcontrols.h
SOURCES += $$PWD/visualtestutil.cpp \
           $$PWD/util.cpp

android|ios {
    DEFINES += QT_QMLTEST_DATADIR=\\\":/data\\\"
} else {
    DEFINES += QT_QMLTEST_DATADIR=\\\"$${_PRO_FILE_PWD_}/data\\\"
}
DEFINES += QQC2_IMPORT_PATH=\\\"$$QQC2_SOURCE_TREE/src/imports\\\"
