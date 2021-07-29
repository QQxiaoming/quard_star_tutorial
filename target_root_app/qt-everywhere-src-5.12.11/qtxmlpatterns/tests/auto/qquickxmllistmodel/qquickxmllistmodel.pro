CONFIG += testcase
TARGET = tst_qquickxmllistmodel
macos:CONFIG -= app_bundle

SOURCES += tst_qquickxmllistmodel.cpp \
           $$PWD/../../../src/imports/xmllistmodel/qqmlxmllistmodel.cpp
HEADERS +=  $$PWD/../../../src/imports/xmllistmodel/qqmlxmllistmodel_p.h

INCLUDEPATH *= $$PWD/../../../src/imports/xmllistmodel

include (../../shared/util.pri)

TESTDATA = data/*

QT += core-private gui-private  qml-private network testlib xmlpatterns

OTHER_FILES += \
    data/groups.qml
