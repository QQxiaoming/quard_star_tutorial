include(../tests.pri)
QT += webengine
HEADERS += proxyserver.h
SOURCES += proxyserver.cpp

# QTBUG-71229
xgd_desktop.name=XDG_CURRENT_DESKTOP
xgd_desktop.value=KDE
QT_TOOL_ENV += xgd_desktop

kde_home.name=KDEHOME
kde_home.value=$$OUT_PWD
QT_TOOL_ENV += kde_home

PROXY_CONFIG= \
    "[Proxy Settings]" \
    "Proxy Config Script=$$PWD/proxy.pac" \
    "ProxyType=2"

mkpath($$OUT_PWD/share/config)
KDE_FILE = $$OUT_PWD/share/config/kioslaverc

!build_pass {
    write_file($$KDE_FILE, PROXY_CONFIG)
}

QMAKE_DISTCLEAN += $$KDE_FILE

