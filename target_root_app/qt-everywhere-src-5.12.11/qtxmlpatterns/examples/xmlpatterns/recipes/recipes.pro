QT          += widgets xmlpatterns
FORMS       += forms/querywidget.ui
HEADERS     = querymainwindow.h ../shared/xmlsyntaxhighlighter.h
RESOURCES   = recipes.qrc
SOURCES     = main.cpp querymainwindow.cpp ../shared/xmlsyntaxhighlighter.cpp
INCLUDEPATH += ../shared/

target.path = $$[QT_INSTALL_EXAMPLES]/xmlpatterns/recipes
INSTALLS += target
