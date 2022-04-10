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
    include/microtone/microtone_platform.hpp \
    include/microtone/exception.hpp \
    include/microtone/midi_input.hpp \
    include/microtone/synthesizer/audio_buffer.hpp \
    include/microtone/synthesizer/envelope.hpp \
    include/microtone/synthesizer/filter.hpp \
    include/microtone/synthesizer/oscillator.hpp \
    include/microtone/synthesizer/synthesizer.hpp \
    include/microtone/synthesizer/synthesizer_voice.hpp \
    include/microtone/synthesizer/wavetable.hpp \
    include/microtone/synthesizer/weighted_wavetable.hpp

HEADERS += \
    src/log.hpp

SOURCES += \
    src/microtone_platform.cpp \
    src/exception.cpp \
    src/log.cpp \
    src/midi_input.cpp \
    src/synthesizer/envelope.cpp \
    src/synthesizer/filter.cpp \
    src/synthesizer/oscillator.cpp \
    src/synthesizer/synthesizer.cpp \
    src/synthesizer/synthesizer_voice.cpp

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

