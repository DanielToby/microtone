TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    Microtone \
    Asciiboard

Asciiboard.depends = \
    Microtone

