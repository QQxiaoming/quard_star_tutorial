CXX_MODULE = webview
TARGET  = declarative_webview
TARGETPATH = QtWebView
IMPORT_VERSION = 1.1

QT += qml quick webview-private
SOURCES += \
    $$PWD/webview.cpp

DYNAMIC_QMLDIR = \
    "module QtWebView" \
    "plugin declarative_webview" \
    "typeinfo plugins.qmltypes" \
    "classname QWebViewModule"
qtHaveModule(webengine):DYNAMIC_QMLDIR += "depends QtWebEngine 1.0"
load(qml_plugin)

OTHER_FILES += qmldir
