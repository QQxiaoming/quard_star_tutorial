SOURCES += main.cpp filetree.cpp mainwindow.cpp ../shared/xmlsyntaxhighlighter.cpp
HEADERS += filetree.h  mainwindow.h ../shared/xmlsyntaxhighlighter.h
FORMS += forms/mainwindow.ui
QT += widgets xmlpatterns
RESOURCES += queries.qrc
INCLUDEPATH += ../shared/

target.path = $$[QT_INSTALL_EXAMPLES]/xmlpatterns/filetree
INSTALLS += target
