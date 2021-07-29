TARGET = qtwebview_darwin

PLUGIN_TYPE = webview
PLUGIN_CLASS_NAME = QDarwinWebViewPlugin
load(qt_plugin)

QT += core gui webview-private
LIBS_PRIVATE += -framework Foundation -framework WebKit
macos: LIBS_PRIVATE += -framework AppKit
ios: LIBS_PRIVATE += -framework UIKit

HEADERS += \
    qdarwinwebview_p.h

SOURCES += \
    qdarwinwebviewplugin.cpp

OBJECTIVE_SOURCES += \
    qdarwinwebview.mm

DISTFILES += \
    darwin.json
