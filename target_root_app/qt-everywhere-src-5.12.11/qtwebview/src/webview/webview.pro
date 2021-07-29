load(qt_build_config)

TARGET = QtWebView

QT_FOR_PRIVATE = quick-private core-private gui-private

MODULE_PLUGIN_TYPES = webview

PUBLIC_HEADERS += \
    qwebview_global.h \
    qtwebviewfunctions.h

PRIVATE_HEADERS += \
    qwebview_p.h \
    qwebviewinterface_p.h \
    qquickwebview_p.h \
    qnativeviewcontroller_p.h \
    qabstractwebview_p.h \
    qquickviewcontroller_p.h \
    qwebviewloadrequest_p.h \
    qquickwebviewloadrequest_p.h \
    qwebviewplugin_p.h \
    qwebviewfactory_p.h

SOURCES += \
    qtwebviewfunctions.cpp \
    qwebview.cpp \
    qquickwebview.cpp \
    qquickviewcontroller.cpp \
    qquickwebviewloadrequest.cpp \
    qwebviewloadrequest.cpp \
    qwebviewplugin.cpp \
    qwebviewfactory.cpp

QMAKE_DOCS = \
             $$PWD/doc/qtwebview.qdocconf

ANDROID_BUNDLED_JAR_DEPENDENCIES = \
    jar/QtAndroidWebView.jar
ANDROID_PERMISSIONS = \
    android.permission.ACCESS_FINE_LOCATION
ANDROID_LIB_DEPENDENCIES = \
    plugins/webview/libqtwebview_android.so

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

load(qt_module)
