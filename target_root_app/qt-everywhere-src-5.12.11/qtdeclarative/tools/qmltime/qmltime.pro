TEMPLATE = app
TARGET = qmltime
QT += qml quick
QT += quick-private
macx:CONFIG -= app_bundle

QMAKE_TARGET_DESCRIPTION = QML Time
SOURCES += qmltime.cpp
