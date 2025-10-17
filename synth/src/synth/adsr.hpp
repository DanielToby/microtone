#pragma once

namespace synth {

struct ADSR {
    double attack;
    double decay;
    double sustain;
    double release;

    ADSR& operator=(const ADSR& other) {
        attack = other.attack;
        decay = other.decay;
        sustain = other.sustain;
        release = other.release;
        return *this;
    }

    [[nodiscard]] bool operator==(const ADSR& other) const {
        return attack == other.attack && decay == other.decay && sustain == other.sustain && release == other.release;
    }

    [[nodiscard]] bool operator!=(const ADSR& other) const { return !(*this == other); }
};

}