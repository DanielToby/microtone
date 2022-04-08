TEMPLATE = app
CONFIG += \
    console \
    c++17

CONFIG -= qt gui

DEFINES += \
    FMT_HEADER_ONLY

macx {
DEFINES += __MACOSX_CORE__
}
windows {
DEFINES += __WINDOWS_MM__
}

HEADERS += \
  asciiboard.hpp \

SOURCES += \
  asciiboard.cpp \
  main.cpp 

CONFIG(macx, macx|win32) {
    PLATFORM = macos
} else {
    PLATFORM = windows
}

INCLUDEPATH += \
  $$PWD/../Microtone/include \
  $$PWD/vendor/ftxui-2.0.0/include/$$PLATFORM \
  $$PWD/../vendor/fmt-8.0.1/include

DEPENDPATH += \
  $$PWD/vendor/ftxui-2.0.0/include$$PLATFORM \
  $$PWD/../vendor/fmt-8.0.1/include


macx {
LIBS += \
    -L$$OUT_PWD/../Microtone -lMicrotone \
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

#RELEASE / DEBUG
CONFIG(debug, debug|release) {
    DEST_DIR = debug
} else {
    DEST_DIR = release
}

LIBS += \
    -L$$OUT_PWD/../Microtone/$$DEST_DIR/ -lMicrotone \
    -L$$PWD/vendor/ftxui-2.0.0/lib/windows -lftxui-component -lftxui-dom -lftxui-screen \
    -L$$PWD/../Microtone/vendor/rtmidi-5.0.0/lib/windows -lrtmidi \
    -L$$PWD/../Microtone/vendor/portaudio-19.7.0/lib/windows -lportaudio_x64

#DEPLOYED FILES
PORTAUDIO.commands = $$quote(XCOPY "$$shell_path($$PWD/../Microtone/vendor/rtmidi-5.0.0/lib/windows/rtmidi.dll)" "$$shell_path($$DEST_DIR)" /c /s /d /y)
RTMIDI.commands = $$quote(XCOPY "$$shell_path($$PWD/../Microtone/vendor/portaudio-19.7.0/lib/windows/portaudio_x64.dll)" "$$shell_path($$DEST_DIR)" /c /s /d /y)

QMAKE_EXTRA_TARGETS += PORTAUDIO RTMIDI
POST_TARGETDEPS += PORTAUDIO RTMIDI
}



