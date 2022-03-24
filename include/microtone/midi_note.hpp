#pragma once

namespace microtone {

struct MidiNote {
    int note;
    int velocity;
    double startTimeStamp;
    double finishTimeStamp;
    bool isActive;


    MidiNote() :
        note{-1},
        velocity{-1},
        startTimeStamp{-1},
        finishTimeStamp{-1},
        isActive{true} {}

    MidiNote(int note, int velocity, double startTimeStamp) :
        note{note},
        velocity{velocity},
        startTimeStamp{startTimeStamp},
        finishTimeStamp{-1},
        isActive{true} {}
};

}
