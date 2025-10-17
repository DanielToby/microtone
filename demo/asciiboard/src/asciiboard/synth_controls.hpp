#pragma once

#include <synth/adsr.hpp>

namespace asciiboard {

struct SynthControls {
    synth::ADSR adsr{0, 0, 0, 0};
    double sineWeight{0};
    double squareWeight{0};
    double triangleWeight{0};
};

}
