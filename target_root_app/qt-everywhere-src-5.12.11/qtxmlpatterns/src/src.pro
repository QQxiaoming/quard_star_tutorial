TEMPLATE = subdirs
SUBDIRS +=  xmlpatterns

qtHaveModule(qml){
    SUBDIRS += imports
    imports.depends = xmlpatterns
}

