include($$QTWEBENGINE_OUT_ROOT/src/core/qtwebenginecore-config.pri)
QT_FOR_CONFIG += webenginecore-private

TARGET = QtWebEngineWidgets

# For our export macros
DEFINES += QT_BUILD_WEBENGINEWIDGETS_LIB

QT += webenginecore widgets network quick
QT_PRIVATE += quick-private gui-private core-private widgets-private quickwidgets webenginecore-private

INCLUDEPATH += $$PWD api ../core ../core/api ../webengine/api

SOURCES = \
        api/qtwebenginewidgetsglobal.cpp \
        api/qwebenginecertificateerror.cpp \
        api/qwebengineclientcertificateselection.cpp \
        api/qwebenginecontextmenudata.cpp \
        api/qwebenginedownloaditem.cpp \
        api/qwebenginefullscreenrequest.cpp \
        api/qwebenginehistory.cpp \
        api/qwebenginepage.cpp \
        api/qwebengineprofile.cpp \
        api/qwebenginescript.cpp \
        api/qwebenginescriptcollection.cpp \
        api/qwebenginesettings.cpp \
        api/qwebengineview.cpp \
        render_widget_host_view_qt_delegate_widget.cpp

HEADERS = \
        api/qtwebenginewidgetsglobal.h \
        api/qwebenginecertificateerror.h \
        api/qwebengineclientcertificateselection.h \
        api/qwebenginecontextmenudata.h \
        api/qwebenginedownloaditem.h \
        api/qwebenginedownloaditem_p.h \
        api/qwebenginefullscreenrequest.h \
        api/qwebenginehistory.h \
        api/qwebenginepage.h \
        api/qwebenginepage_p.h \
        api/qwebengineprofile.h \
        api/qwebengineprofile_p.h \
        api/qwebenginescriptcollection.h \
        api/qwebenginescriptcollection_p.h \
        api/qwebenginesettings.h \
        api/qwebengineview.h \
        api/qwebengineview_p.h \
        render_widget_host_view_qt_delegate_widget.h

qtConfig(webengine-printing-and-pdf) {
    QT += printsupport
}

load(qt_module)
