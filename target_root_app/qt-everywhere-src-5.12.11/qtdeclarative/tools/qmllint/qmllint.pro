option(host_build)

QT = core qmldevtools-private

SOURCES += main.cpp

QMAKE_TARGET_DESCRIPTION = QML Syntax Verifier

load(qt_tool)
