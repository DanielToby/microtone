TEMPLATE = lib
CONFIG += \
    staticlib \
    c++17

CONFIG -= qt gui

DEFINES += \
    FMT_HEADER_ONLY \
    SPDLOG_FMT_EXTERNAL

macx {
DEFINES += __MACOSX_CORE__
}
windows {
DEFINES += __WINDOWS_MM__
}

HEADERS += \
    include/microtone/envelope.hpp \
    include/microtone/exception.hpp \
    include/microtone/filter.hpp \
    include/microtone/microtone_platform.hpp \
    include/microtone/midi_input.hpp \
    include/microtone/oscillator.hpp \
    include/microtone/synthesizer.hpp \
    include/microtone/synthesizer_voice.hpp

HEADERS += \
    src/log.hpp

SOURCES += \
    src/envelope.cpp \
    src/exception.cpp \
    src/filter.cpp \
    src/log.cpp \
    src/microtone_platform.cpp \
    src/midi_input.cpp \
    src/oscillator.cpp \
    src/synthesizer.cpp \
    src/synthesizer_voice.cpp

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/../vendor/fmt-8.0.1/include \
    $$PWD/vendor/portaudio-19.7.0/include \
    $$PWD/vendor/rtmidi-5.0.0/include \
    $$PWD/vendor/spdlog-1.9.0/include

DEPENDPATH += \
    $$PWD/include \
    $$PWD/../vendor/fmt-8.0.1/include \
    $$PWD/vendor/portaudio-19.7.0/include \
    $$PWD/vendor/spdlog-1.9.0/include \
    $$PWD/vendor/rtmidi-5.0.0/include

macx {
LIBS += \
    -L$$PWD/vendor/rtmidi-5.0.0/lib/macos -lrtmidi \
    -L$$PWD/vendor/portaudio-19.7.0/lib/macos -lportaudio \
    -framework AudioToolbox \
    -framework Carbon \
    -framework CoreAudio \
    -framework CoreFoundation \
    -framework CoreServices \
    -framework CoreMIDI
}

win32 {
LIBS += \
    -L$$PWD/vendor/rtmidi-5.0.0/lib/windows -lrtmidi \
    -L$$PWD/vendor/portaudio-19.7.0/lib/windows -lportaudio_x64
}

