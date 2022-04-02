TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Microtone \
    Display

Display.depends = \
    Microtone

