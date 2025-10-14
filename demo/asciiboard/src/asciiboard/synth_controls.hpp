#pragma once

namespace asciiboard {

struct SynthControls {
    double attack{0};
    double decay{0};
    double sustain{0};
    double release{0};
    double sineWeight{0};
    double squareWeight{0};
    double triangleWeight{0};
};

}
