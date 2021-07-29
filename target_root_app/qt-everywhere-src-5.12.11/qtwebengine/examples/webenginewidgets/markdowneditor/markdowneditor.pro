TEMPLATE = app

QT += webenginewidgets webchannel

HEADERS += \
    mainwindow.h \
    previewpage.h \
    document.h

SOURCES = \
    main.cpp \
    mainwindow.cpp \
    previewpage.cpp \
    document.cpp

RESOURCES = \
    resources/markdowneditor.qrc

# Disable Qt Quick compiler because the example doesn't use QML, but more importantly so that
# the source code of the .js files is not removed from the embedded qrc file.
CONFIG -= qtquickcompiler

FORMS += \
    mainwindow.ui

DISTFILES += \
    resources/3rdparty/MARKDOWN-LICENSE.txt \
    resources/3rdparty/MARKED-LICENSE.txt

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/markdowneditor
INSTALLS += target
