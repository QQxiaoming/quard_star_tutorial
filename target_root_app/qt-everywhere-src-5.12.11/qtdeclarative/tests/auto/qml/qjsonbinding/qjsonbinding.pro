CONFIG += testcase
TARGET = tst_qjsonbinding
macx:CONFIG -= app_bundle

SOURCES += tst_qjsonbinding.cpp

include (../../shared/util.pri)

# QMAKE_CXXFLAGS = -fprofile-arcs -ftest-coverage
# LIBS += -lgcov

TESTDATA = data/*

QT += core qml testlib gui-private
