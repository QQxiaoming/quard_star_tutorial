include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri) # workaround for QTBUG-68093
QT_FOR_CONFIG += webenginecore-private

include(../tests.pri)
QT *= core-private webenginecore-private

qtConfig(webengine-poppler-cpp) {
    CONFIG += link_pkgconfig
    PKGCONFIG += poppler-cpp
}
