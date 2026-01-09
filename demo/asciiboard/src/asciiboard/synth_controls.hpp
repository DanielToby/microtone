#pragma once

#include <synth/adsr.hpp>

namespace asciiboard {

struct SynthControls {
    synth::ADSR adsr{0, 0, 0, 0};
    double sineWeight{0};
    double squareWeight{0};
    double triangleWeight{0};

    float gain{0.f};
    float lfoFrequencyHz{0.f};
    float lfoGain{0.f};
};

}
