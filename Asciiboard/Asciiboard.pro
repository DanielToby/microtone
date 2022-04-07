TEMPLATE = app
CONFIG += \
    console \
    c++17

CONFIG -= qt gui

DEFINES += \
    FMT_HEADER_ONLY \
    __MACOSX_CORE__

HEADERS += \
  asciiboard.hpp \

SOURCES += \
  asciiboard.cpp \
  main.cpp 

INCLUDEPATH += \
  $$PWD/../Microtone/include \
  $$PWD/vendor/ftxui-2.0.0/include \
  $$PWD/../vendor/fmt-8.0.1/include

DEPENDPATH += \
  $$PWD/vendor/ftxui-2.0.0/include \
  $$PWD/../vendor/fmt-8.0.1/include

LIBS += \
  -L$$OUT_PWD/../Microtone -lMicrotone \
  -L$$PWD/vendor/ftxui-2.0.0/lib -lftxui-component -lftxui-dom -lftxui-screen \
  -L$$PWD/../Microtone/vendor/rtmidi-5.0.0/lib -lrtmidi \
  -L$$PWD/../Microtone/vendor/portaudio-19.7.0/lib -lportaudio \
  -framework AudioToolbox \
  -framework Carbon \
  -framework CoreAudio \
  -framework CoreFoundation \
  -framework CoreServices \
  -framework CoreMIDI

