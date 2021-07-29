TARGET = qtwebview_android

PLUGIN_TYPE = webview
PLUGIN_CLASS_NAME = QAndroidWebViewPlugin
load(qt_plugin)

QT += core gui webview-private
LIBS_PRIVATE += -ljnigraphics

HEADERS += \
    qandroidwebview_p.h

SOURCES += \
    qandroidwebviewplugin.cpp \
    qandroidwebview.cpp

OTHER_FILES +=

DISTFILES += \
    android.json
