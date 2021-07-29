TEMPLATE = subdirs
QT_FOR_CONFIG += qml-private

qtConfig(qml-devtools):
    SUBDIRS += qmlmin

qtConfig(private_tests): \
    SUBDIRS += qqmlparser

