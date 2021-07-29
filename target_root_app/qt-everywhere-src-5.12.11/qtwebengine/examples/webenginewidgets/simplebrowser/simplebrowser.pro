TEMPLATE = app
TARGET = simplebrowser
QT += webenginewidgets

HEADERS += \
    browser.h \
    browserwindow.h \
    downloadmanagerwidget.h \
    downloadwidget.h \
    tabwidget.h \
    webpage.h \
    webpopupwindow.h \
    webview.h

SOURCES += \
    browser.cpp \
    browserwindow.cpp \
    downloadmanagerwidget.cpp \
    downloadwidget.cpp \
    main.cpp \
    tabwidget.cpp \
    webpage.cpp \
    webpopupwindow.cpp \
    webview.cpp

FORMS += \
    certificateerrordialog.ui \
    passworddialog.ui \
    downloadmanagerwidget.ui \
    downloadwidget.ui

RESOURCES += data/simplebrowser.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/simplebrowser
INSTALLS += target
