#pragma once

#include <array>

namespace microtone {

const int WAVETABLE_LENGTH = 512;
using WaveTable = std::array<float, WAVETABLE_LENGTH>;

}
