#pragma once

#include <array>

namespace microtone {

const int FRAMES_PER_BUFFER = 512;
using AudioBuffer = std::array<float, FRAMES_PER_BUFFER>;

}
