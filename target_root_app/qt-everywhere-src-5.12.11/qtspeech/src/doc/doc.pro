TEMPLATE = aux

CONFIG += force_qt
QT = core-private

QMAKE_DOCS = $$PWD/qtspeech.qdocconf

OTHER_FILES += \
    $$PWD/src/*.qdoc \
    $$PWD/QtSpeechDoc
