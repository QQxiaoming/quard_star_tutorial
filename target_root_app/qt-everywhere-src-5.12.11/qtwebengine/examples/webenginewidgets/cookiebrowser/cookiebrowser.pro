QT += core gui webenginewidgets
TARGET = cookiebrowser
TEMPLATE = app

SOURCES += \
    main.cpp\
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui \
    cookiedialog.ui \
    cookiewidget.ui

RESOURCES += \
    cookiebrowser.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/cookiebrowser
INSTALLS += target
