TARGET = qtwebview_winrt
QT += core gui webview-private

PLUGIN_TYPE = webview
PLUGIN_CLASS_NAME = QWinrtWebViewPlugin

load(qt_plugin)

NO_PCH_SOURCES += \
    qwinrtwebview.cpp

SOURCES += \
    qwinrtwebviewplugin.cpp

HEADERS += \
    qwinrtwebview_p.h

OTHER_FILES +=

DISTFILES += \
    winrt.json
