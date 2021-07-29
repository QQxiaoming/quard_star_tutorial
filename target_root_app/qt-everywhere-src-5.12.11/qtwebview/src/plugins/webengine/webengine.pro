TARGET = qtwebview_webengine

PLUGIN_TYPE = webview
PLUGIN_CLASS_NAME = QWebEngineWebViewPlugin
load(qt_plugin)

QT += core gui webengine-private webenginecore-private webview-private

HEADERS += \
    qwebenginewebview_p.h

SOURCES += \
    qwebenginewebview.cpp \
    qwebenginewebviewplugin.cpp

OTHER_FILES +=

DISTFILES += \
    webengine.json
