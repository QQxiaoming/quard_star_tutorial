CONFIG += testcase
TARGET = tst_qqmlmetatype
SOURCES += tst_qqmlmetatype.cpp
macx:CONFIG -= app_bundle

TESTDATA = data/*
include (../../shared/util.pri)

qmlfiles.files = data/CompositeType.qml
qmlfiles.prefix = /tstqqmlmetatype
RESOURCES += qmlfiles

qmldirresource.files = \
    data/Components/App.qml \
    data/Components/qmldir \
    data/enumsInRecursiveImport.qml
qmldirresource.prefix = /
RESOURCES += qmldirresource

QT += core-private gui-private qml-private testlib
