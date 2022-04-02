TEMPLATE = app
CONFIG += \
    console \
    c++17

CONFIG -= qt gui

DEFINES += \
    FMT_HEADER_ONLY \
    __MACOSX_CORE__

HEADERS += \
  display.hpp \

SOURCES += \
  display.cpp \
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
  -framework CoreMIDI \
  -framework CoreAudio \
  -framework CoreFoundation

