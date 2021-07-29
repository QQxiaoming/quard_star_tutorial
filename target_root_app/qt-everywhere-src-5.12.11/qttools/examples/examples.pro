TEMPLATE = subdirs
qtHaveModule(widgets): SUBDIRS += help designer linguist uitools assistant

!qtConfig(process): SUBDIRS -= assistant designer

qtNomakeTools( \
    assistant \
)
