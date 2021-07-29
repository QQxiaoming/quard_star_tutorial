TEMPLATE = subdirs
SUBDIRS += \
    buttons \
    gifs \
    fonts \
    screenshots \
    styles \
    testbench

qtConfig(systemtrayicon): SUBDIRS += systemtrayicon

qtHaveModule(widgets): SUBDIRS += viewinqwidget
