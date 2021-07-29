requires(contains(QT_CONFIG, private_tests))

SOURCES += tst_qnearfieldtagtype1.cpp
TARGET = tst_qnearfieldtagtype1
CONFIG += testcase

QT = core nfc-private testlib

INCLUDEPATH += ../../../src/nfc
VPATH += ../../../src/nfc

HEADERS += \
    qnearfieldmanagervirtualbase_p.h \
    qnearfieldmanager_emulator_p.h \
    qnearfieldtarget_emulator_p.h \
    targetemulator_p.h

SOURCES += \
    qnearfieldmanagervirtualbase.cpp \
    qnearfieldmanager_emulator.cpp \
    qnearfieldtarget_emulator.cpp \
    targetemulator.cpp

DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"

TESTDATA += "$$PWD/../nfcdata/Qt Labs Website Tag Type1.nfc" \
            "$$PWD/../nfcdata/Qt Website Tag Type1.nfc"

builtin_testdata: DEFINES += BUILTIN_TESTDATA
