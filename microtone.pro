TEMPLATE = app
CONFIG += console
CONFIG += c++17
CONFIG -= qt gui

DEFINES += \
    FMT_HEADER_ONLY \
    SPDLOG_FMT_EXTERNAL \
    __MACOSX_CORE__

HEADERS += \
    include/microtone/exception.hpp \
    include/microtone/log.hpp \
    include/microtone/midi_input.hpp \
    include/microtone/synthesizer.hpp

SOURCES += \
    main.cpp \
    src/exception.cpp \
    src/log.cpp \
    src/midi_input.cpp \
    src/synthesizer.cpp

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/vendor/fmt-8.0.1/include \
    $$PWD/vendor/libstdaudio-0.0.2/include \
    $$PWD/vendor/rtmidi-5.0.0/include \
    $$PWD/vendor/spdlog-1.9.0/include

DEPENDPATH += \
    $$PWD/include \
    $$PWD/vendor/fmt-8.0.1/include \
    $$PWD/vendor/libstdaudio-0.0.2/include \
    $$PWD/vendor/spdlog-1.9.0/include \
    $$PWD/vendor/rtmidi-5.0.0/include

LIBS += \
    -L$$PWD/vendor/rtmidi-5.0.0/lib -lrtmidi \
    -framework CoreMIDI \
    -framework CoreAudio \
    -framework CoreFoundation

