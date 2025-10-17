#pragma once

#include <unordered_set>
#include <vector>

#include <synth/voice.hpp>

namespace synth {

struct MidiMessage {
    int status;
    int note;
    int velocity;
};

struct MidiDevice {
    MidiDevice() = default;
    explicit MidiDevice(const std::vector<Voice>& voices) : voices(voices), sustainPedalOn(false) {}

    void noteOn(int note, int velocity) {
        voices[note].setVelocity(velocity);
        voices[note].triggerOn();
        activeVoices.insert(note);
    }

    void noteOff(int note) {
        if (sustainPedalOn) {
            sustainedVoices.insert(note);
        } else {
            voices[note].triggerOff();
        }
    }

    void setSustain(bool sustain) {
        sustainPedalOn = sustain;
        if (!sustainPedalOn) {
            for (const auto& id : sustainedVoices) {
                voices[id].triggerOff();
            }
            sustainedVoices.clear();
        }
    }

    std::vector<Voice> voices;
    std::unordered_set<int> activeVoices;
    std::unordered_set<int> sustainedVoices;
    bool sustainPedalOn;
};

}