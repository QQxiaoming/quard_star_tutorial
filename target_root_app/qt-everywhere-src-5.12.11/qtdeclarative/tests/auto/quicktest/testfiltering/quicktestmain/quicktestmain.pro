CONFIG += qmltestcase
macos:CONFIG -= app_bundle
TARGET = quicktestmain

DEFINES += QT_QMLTEST_DATADIR=\\\"$${PWD}\\\"

SOURCES += quicktestmain.cpp

TESTDATA += $$PWD/*.qml

DESTDIR = ./
