requires(qtHaveModule(sql))
TEMPLATE = subdirs

SUBDIRS += \
           help \
           assistant \
           qhelpgenerator \
           qcollectiongenerator

assistant.depends = help
qhelpgenerator.depends = help

qtNomakeTools( \
    assistant \
    qhelpgenerator \
    qcollectiongenerator \
)
