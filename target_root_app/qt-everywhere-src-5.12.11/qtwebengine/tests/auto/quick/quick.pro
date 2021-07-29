include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore-private

TEMPLATE = subdirs

SUBDIRS += \
    dialogs \
    inspectorserver \
    publicapi \
    qquickwebenginedefaultsurfaceformat \
    qquickwebengineview \
    qtbug-70248

qtConfig(webengine-testsupport) {
    SUBDIRS += \
        qmltests \
        qquickwebengineviewgraphics
}

# QTBUG-66055
boot2qt: SUBDIRS -= inspectorserver qquickwebenginedefaultsurfaceformat qquickwebengineview qmltests dialogs qtbug-70248
